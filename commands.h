
#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>

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

    static void uci() {
        return;
    }

    static void debug() {
        return;
    }

    static void isready() {
        return;
    }

    static void setoption() {
        return;
    }

    static void register_engine() {
        return;
    }

    static void ucinewgame() {
        return;
    }

    static void position() {
        return;
    }

    static void go() {
        return;
    }

    static void stop() {
        return;
    }

    static void ponderhit() {
        return;
    }

    static void quit() {
        return;
    }

} // namespace COM

#endif
