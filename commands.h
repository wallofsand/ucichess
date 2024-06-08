
#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <iostream>

namespace COM {
    std::string commands[] = {
        "uci",
        "debug",
        "isready",
        "setoption",
        "register",
        "ucinewgame",
        "position",
        "go",
        "stop",
        "ponderhit",
        "quit",
    };

    std::string go_commands[] {
        "searchmoves",
        "ponder",
        "wtime",
        "btime",
        "winc",
        "binc",
        "movestogo",
        "depth",
        "nodes",
        "mate",
        "movetime",
        "infinite",
    };

    int take_command(int args, char** argv) {
        for (int i = 1; i < args; i++) {
            std::string instr(*(argv + i));
            std::cout << i << ": " << instr << std::endl;
        }
        return 0;
    }

    void uci();
    void debug();
    void isready();
    void setoption();
    void register_engine();
    void ucinewgame();
    void position();
    void go();
    void stop();
    void ponderhit();
    void quit();

    void proc(std::string instr) {
        if (instr == "uci")        return uci();
        if (instr == "debug")      return debug();
        if (instr == "isready")    return isready();
        if (instr == "setoption")  return setoption();
        if (instr == "register")   return register_engine();
        if (instr == "ucinewgame") return ucinewgame();
        if (instr == "position")   return position();
        if (instr == "go")         return go();
        if (instr == "stop")       return stop();
        if (instr == "ponderhit")  return ponderhit();
        if (instr == "quit")       return quit();
    }

    void uci() {
        return;
    }

    void debug() {
        return;
    }

    void isready() {
        return;
    }

    void setoption() {
        return;
    }

    void register_engine() {
        return;
    }

    void ucinewgame() {
        return;
    }

    void position() {
        return;
    }

    void go() {
        return;
    }

    void stop() {
        return;
    }

    void ponderhit() {
        return;
    }

    void quit() {
        return;
    }

    void loop() {
        bool quit = false;
        while (!quit) {
            
        }
    }

} // namespace COM

#endif
