// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ConstellationInfo.h"

using namespace Eigen;
using namespace std;


vector<ConstellationInfo> ConstellationInfo::s_constellations;

struct ConstellationRecord
{
    const char* name;
    float rightAscension;
    float declination;
};


static ConstellationRecord StandardConstellations[] =
{
    { "Andromeda", 0.807667f, 37.431833f },
    { "Antlia", 10.273833f, -32.483500f },
    { "Apus", 16.144167f, -75.300000f },
    { "Aquarius", 22.289667f, -10.789167f },
    { "Aquila", 19.667000f, 3.410833f },
    { "Ara", 17.374833f, -56.588333f },
    { "Aries", 2.636000f, 20.792333f },
    { "Auriga", 6.073667f, 42.028000f },
    { "Bootes", 14.710667f, 31.202667f },
    { "Caelum", 4.704500f, -37.881667f },
    { "Camelopardalis", 8.856167f, 69.381500f },
    { "Cancer", 8.649333f, 19.805833f },
    { "Canes Venatici", 13.116000f, 40.101833f },
    { "Canis Major", 6.829000f, -22.140333f },
    { "Canis Minor", 7.652833f, 6.427167f },
    { "Capricornus", 21.048833f, -18.023167f },
    { "Carina", 8.695000f, -63.219333f },
    { "Cassiopeia", 1.319333f, 62.184000f },
    { "Centaurus", 13.071167f, -47.345333f },
    { "Cepheus", 2.544000f, 71.008500f },
    { "Cetus", 1.668333f, -7.179333f },
    { "Chamaeleon", 10.692167f, -79.205000f },
    { "Circinus", 14.575667f, -63.030333f },
    { "Columba", 5.862667f, -35.094500f },
    { "Coma Berenices", 12.787833f, 23.305667f },
    { "Corona Australis", 18.646500f, -41.147500f },
    { "Corona Borealis", 15.843167f, 32.624833f },
    { "Corvus", 12.442000f, -18.436667f },
    { "Crater", 11.395833f, -15.929000f },
    { "Crux", 12.449833f, -60.186500f },
    { "Cygnus", 20.588000f, 44.545000f },
    { "Delphinus", 20.693500f, 11.671000f },
    { "Dorado", 5.241833f, -59.387000f },
    { "Draco", 15.144000f, 67.006667f },
    { "Equuleus", 21.187667f, 7.758167f },
    { "Eridanus", 3.300333f, -28.756167f },
    { "Fornax", 2.798000f, -31.634500f },
    { "Gemini", 7.070667f, 22.600167f },
    { "Grus", 22.456500f, -46.351833f },
    { "Hercules", 17.386000f, 27.498833f },
    { "Horologium", 3.276000f, -53.336333f },
    { "Hydra", 11.612167f, -14.531833f },
    { "Hydrus", 2.344167f, -69.956500f },
    { "Indus", 21.972167f, -59.706667f },
    { "Lacerta", 22.461333f, 46.041833f },
    { "Leo", 10.667167f, 13.138667f },
    { "Leo Minor", 10.245333f, 32.134667f },
    { "Lepus", 5.565833f, -19.046333f },
    { "Libra", 15.199333f, -15.234667f },
    { "Lupus", 15.220167f, -42.708833f },
    { "Lynx", 7.992167f, 47.466667f },
    { "Lyra", 18.852833f, 36.689333f },
    { "Mensa", 5.415000f, -77.504000f },
    { "Microscopium", 20.964667f, -36.274833f },
    { "Monoceros", 7.060500f, 0.282167f },
    { "Musca", 12.588000f, -70.161000f },
    { "Norma", 15.903000f, -51.351500f },
    { "Octans", 23.000000f, -82.152000f },
    { "Ophiuchus", 17.394833f, -7.912333f },
    { "Orion", 5.576500f, 5.949000f },
    { "Pavo", 19.611833f, -65.781500f },
    { "Pegasus", 22.697333f, 19.466333f },
    { "Perseus", 3.175000f, 45.013167f },
    { "Phoenix", 0.931833f, -48.580667f },
    { "Pictor", 5.707667f, -53.474167f },
    { "Pisces", 0.482833f, 13.687167f },
    { "Piscis Austrinus", 22.284500f, -30.642167f },
    { "Puppis", 7.258000f, -31.177333f },
    { "Pyxis", 8.952667f, -27.351667f },
    { "Reticulum", 3.921167f, -59.997500f },
    { "Sagitta", 19.650833f, 18.861333f },
    { "Sagittarius", 19.099000f, -28.476833f },
    { "Scorpius", 16.887333f, -27.031500f },
    { "Sculptor", 0.438000f, -32.088333f },
    { "Scutum", 18.673167f, -9.888667f },
    { "Serpens (Caput)", 15.774833f, 10.970000f },
    { "Serpens (Cauda)", 18.126667f, -4.862167f },
    { "Sextans", 10.271500f, -2.614667f },
    { "Taurus", 4.702167f, 14.877167f },
    { "Telescopium", 19.325667f, -51.036833f },
    { "Triangulum", 2.184500f, 31.476000f },
    { "Triangulum Australe", 16.082500f, -65.388000f },
    { "Tucana", 23.777333f, -65.830000f },
    { "Ursa Major", 11.312667f, 50.721167f },
    { "Ursa Minor", 15.000000f, 77.699833f },
    { "Vela", 9.577333f, -47.167167f },
    { "Virgo", 13.406500f, -4.158500f },
    { "Volans", 7.795500f, -69.801167f },
    { "Vulpecula", 20.231333f, 24.442667f }
};


const vector<ConstellationInfo>&
ConstellationInfo::constellations()
{
    if (s_constellations.empty())
    {
        unsigned int count = sizeof(StandardConstellations) / sizeof(StandardConstellations[0]);
        for (unsigned int i = 0; i < count; ++i)
        {
            const ConstellationRecord& c = StandardConstellations[i];
            ConstellationInfo info(c.name);
            info.setLabelLocation(Vector2f(c.rightAscension, c.declination));
            s_constellations.push_back(info);
        }
    }

    return s_constellations;
}
