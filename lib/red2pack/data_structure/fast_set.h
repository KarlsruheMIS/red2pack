/******************************************************************************
 * fast_set.h from KaHIP
 *****************************************************************************/

#ifndef RED2PACK_FAST_SET_H
#define RED2PACK_FAST_SET_H

#include <vector>

namespace red2pack {
class fast_set {
        std::vector<int> used;
        int uid;

       public:
        fast_set(int const n) : used(n, 0), uid(1) {}

        void clear() {
                uid++;
                if (uid < 0) {
                        for (unsigned int i = 0; i < used.size(); i++) used[i] = 0;
                        uid = 1;
                }
        }

        bool add(int i) {
                bool const res(used[i] != uid);
                used[i] = uid;
                return res;
        }

        void remove(int i) { used[i] = uid - 1; }

        bool get(int i) const { return (used[i] == uid); }
};
}

#endif // RED2PACK_FAST_SET_H
