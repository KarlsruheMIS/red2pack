//
// Created by Jannick Borowitz on 20.06.24.
// Scoped Timer based on scoped timer class by D. Seemaier (dKaMinPar)
//

#ifndef RED2PACK_SCOPED_TIMER_H
#define RED2PACK_SCOPED_TIMER_H

#include <string>

#include "m2s_log.h"
#include "timer.h"

namespace red2pack {

#define RED2PACK_SCOPED_TIMER(name) \
        auto red2pack_scoped_timer##__LINE__ = scoped_timer(name);

class scope_timer_hierarchy {
       public:
        static scope_timer_hierarchy* instance() {
                static scope_timer_hierarchy instance;
                return &instance;
        }
        unsigned incr() {
                return ++current_level;
        }
        unsigned decr() {
                return --current_level;
        }
        unsigned level() {
                return current_level;
        }
       private:
        unsigned current_level = 0;
};

class scoped_timer {
       public:
        explicit scoped_timer(std::string name): name(std::move(name)) {
                scope_timer_hierarchy::instance()->incr();
                t.restart();
        }

        inline ~scoped_timer();

       private:
        timer t;
        std::string name;

};
scoped_timer::~scoped_timer() {
        scope_timer_hierarchy::instance()->decr();
        m2s_log::instance()->log_time(scope_timer_hierarchy::instance()->level(), name, t.elapsed());
}

}

#endif  // RED2PACK_SCOPED_TIMER_H
