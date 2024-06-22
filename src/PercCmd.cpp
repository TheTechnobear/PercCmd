
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>

#include "cJSON.h"

void log(const std::string& m) {
    std::ofstream s("/dev/kmsg");
    s << "PercCmd : " << m << std::endl;
}

void error(const std::string& m) {
    log("ERROR: " + m);
}


// hardware implementation
class Hardware {
public:
    Hardware() = default;
    virtual ~Hardware();
    bool init();
    int getButtonState();

private:
    int buttonFD_ = -1;
#ifdef TARGET_XMX
    bool map(char* keyMap, int btn) {
        int code = 59 + btn; 
        return keyMap[code / 8] & (1 << (code % 8)); }
#else
    bool map(char* keyMap, int btn) {
        if (btn == 0)
            return keyMap[11] & 0b00000001;
        else if (btn == 1)
            return keyMap[10] & 0b10000000;
        else if (btn == 2)
            return keyMap[8] & 0b00010000;
        else if (btn == 3)
            return keyMap[8] & 0b00001000;
        else if (btn == 4)
            return keyMap[8] & 0b00000001;
        else if (btn == 5)
            return keyMap[7] & 0b10000000;
        else if (btn == 6)
            return keyMap[7] & 0b01000000;
        else if (btn == 7)
            return keyMap[7] & 0b00100000;
        return false;
    }
#endif
};


Hardware::~Hardware() {
    if (buttonFD_ >= 0) close(buttonFD_);
}

bool Hardware::init() {
    // open the buttons
    buttonFD_ = open("/dev/input/by-path/platform-matrix-keypad-event", O_RDONLY | O_NONBLOCK);
    if (buttonFD_ < 0) {
        error("hardware: failed to open button");
        return false;
    }

    return true;
}

int Hardware::getButtonState() {
    const int MAX_KEYS = 120;
    const int MAP_SIZE = MAX_KEYS / 8 + 1;

    char key_map[MAP_SIZE];  //  Create a byte array the size of the number of keys
    for (int i = 0; i < MAP_SIZE; i++) key_map[i] = 0;

    if (ioctl(buttonFD_, EVIOCGKEY(sizeof(key_map)), key_map) < 0) { return -1; }

    int mask = 0;
    for (int i = 0; i < 8; i++) {
        if (map(key_map, i)) { mask |= (1 << i); }
    }
    return mask;
}


int pid = 0;
bool keepRunning = true;

void intHandler(int sig) {
    // only called in main thread
    if (sig == SIGINT) {
        log("Received SIGINT");
        keepRunning = 0;
        if (pid > 0) kill(pid, SIGKILL);
    }
}

int main(int argc, char** argv) {
    log("Starting");
    Hardware hw;

    std::string command = "./Synthor";
    std::string wd = "/media/BOOT";


    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        error("Error calling pthread_setaffinity_np: " + std::to_string(rc));
    } else {
        log("Set affinity for main thread");
    }

    // block sigint from other threads
    sigset_t sigset, oldset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sigset, &oldset);

    // Install the signal handler for SIGINT.
    struct sigaction s;
    s.sa_handler = intHandler;
    sigemptyset(&s.sa_mask);
    s.sa_flags = 0;
    sigaction(SIGINT, &s, NULL);

    // Restore the old signal mask only for this thread.
    pthread_sigmask(SIG_SETMASK, &oldset, NULL);


    keepRunning = true;

    int cmd = 0;

    if (hw.init()) {
        log("Hardware init success");
        int btnmask = hw.getButtonState();

        log("Button mask: " + std::to_string(btnmask));
        if (btnmask > 0) {
            for (int i = 0; i < 8; i++) {
                if (btnmask & (1 << i)) {
                    cmd = i;
                    break;
                }
            }
        } else {
            auto lastRunFD = open("./lastBtn", O_RDONLY);
            if (lastRunFD >= 0) {
                auto len = read(lastRunFD, &cmd, 1);
                if (len > 0) { log("Last run button: " + std::to_string(cmd)); }
                close(lastRunFD);
            } else {
                log("No button pressed and no last Btn, run default");
            }
        }
    } else {
        error("Hardware init failed, run default");
    }


    auto confFD = open("./perccmd.json", O_RDONLY);
    if (confFD >= 0) {
        char buffer[1024];
        auto len = read(confFD, buffer, 1024);
        if (len > 0) {
            buffer[len] = 0;
            cJSON* root = cJSON_Parse(buffer);
            if (root) {
                cJSON* jCmds = cJSON_GetObjectItem(root, "commands");
                int num = cJSON_GetArraySize(jCmds);
                if (cmd < num) {
                    cJSON* jObj = cJSON_GetArrayItem(jCmds, cmd);
                    cJSON* jCmd = cJSON_GetObjectItem(jObj, "command");
                    cJSON* jWd = cJSON_GetObjectItem(jObj, "directory");
                    if (jCmd && jWd) {
                        // validate the config values are reasonable
                        bool ok = true;
                        struct stat info;
                        if (stat(jWd->valuestring, &info)) {
                            error("Directory not found: " + std::string(jWd->valuestring));
                            ok = false;
                        } else {
                            if (!(info.st_mode & S_IFDIR)) {
                                error("Directory not valid: " + std::string(jWd->valuestring));
                                ok = false;
                            } else {
                                std::string execFile = std::string(jWd->valuestring) + "/" + jCmd->valuestring;
                                if (access(execFile.c_str(), X_OK) != 0) {
                                    error("Command not executable: " + execFile);
                                    ok = false;
                                }
                            }
                        }
                        if (ok) {
                            // ok, we are good to go, set the command and working directory
                            // and write the last button to ./lastBtn
                            command = jCmd->valuestring;
                            wd = jWd->valuestring;
                            auto lastRunFD = open("./lastBtn", O_CREAT | O_WRONLY | O_TRUNC);
                            if (lastRunFD >= 0) {
                                log("Write last run button: " + std::to_string(cmd));
                                write(lastRunFD, &cmd, 1);
                                close(lastRunFD);
                            }
                        }
                    }
                }
            }
            cJSON_Delete(root);
        }
    }
    close(confFD);

    // we will keep restart the app if it fails
    // we have a graceful shutdown on SIGINT / exit
    while (keepRunning) {
        pid = fork();
        if (pid > 0) {
            // this process
            int status = 0;
            waitpid(pid, &status, 0);
            log("exit code " + std::to_string(status));
            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                if (exit_status == 0) {
                    log("Child process exited normally, exiting");
                } else {
                    sleep(1);
                    log("Child process exited, restarting");
                    keepRunning = true;
                }
            } else {
                sleep(1);
                log("Child process failed, restarting");
            }
            pid = 0;
        } else {
            // new child, i.e app
            chdir(wd.c_str());
            log("Command: " + command + " in " + wd);
            execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
        }
    }

    // been told to stop, block SIGINT, to allow clean termination
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sigset, &oldset);
    log("Shutdown");
    return 0;
}
