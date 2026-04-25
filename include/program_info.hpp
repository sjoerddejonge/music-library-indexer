//
// Created by Sjoerd de Jonge on 24/04/2026.
//

#ifndef MLI_PROGRAM_INFO_H
#define MLI_PROGRAM_INFO_H
#include <string>

/*
 *  Contains the runtime name of the program.
 */

namespace program {
    void init(int argc, char *argv[]);
    const std::string& name();
}

#endif //MLI_PROGRAM_INFO_H
