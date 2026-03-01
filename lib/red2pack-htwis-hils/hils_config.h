#ifndef RED2PACK_HILS_CONFIG_H
#define RED2PACK_HILS_CONFIG_H

namespace red2pack {

struct HilsConfig {

        long rand_seed;

        int iterations; // maximum iteration number

        double p[4]; // intensification/exploration parameters
};

}

#endif  // RED2PACK_HILS_CONFIG_H
