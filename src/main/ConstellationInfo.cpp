// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

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
    { "Andromeda", 0.807667, 37.431833 },
    { "Antlia", 10.273833, -32.483500 },
    { "Apus", 16.144167, -75.300000 },
    { "Aquarius", 22.289667, -10.789167 },
    { "Aquila", 19.667000, 3.410833 },
    { "Ara", 17.374833, -56.588333 },
    { "Aries", 2.636000, 20.792333 },
    { "Auriga", 6.073667, 42.028000 },
    { "Bootes", 14.710667, 31.202667 },
    { "Caelum", 4.704500, -37.881667 },
    { "Camelopardalis", 8.856167, 69.381500 },
    { "Cancer", 8.649333, 19.805833 },
    { "Canes Venatici", 13.116000, 40.101833 },
    { "Canis Major", 6.829000, -22.140333 },
    { "Canis Minor", 7.652833, 6.427167 },
    { "Capricornus", 21.048833, -18.023167 },
    { "Carina", 8.695000, -63.219333 },
    { "Cassiopeia", 1.319333, 62.184000 },
    { "Centaurus", 13.071167, -47.345333 },
    { "Cepheus", 2.544000, 71.008500 },
    { "Cetus", 1.668333, -7.179333 },
    { "Chamaeleon", 10.692167, -79.205000 },
    { "Circinus", 14.575667, -63.030333 },
    { "Columba", 5.862667, -35.094500 },
    { "Coma Berenices", 12.787833, 23.305667 },
    { "Corona Australis", 18.646500, -41.147500 },
    { "Corona Borealis", 15.843167, 32.624833 },
    { "Corvus", 12.442000, -18.436667 },
    { "Crater", 11.395833, -15.929000 },
    { "Crux", 12.449833, -60.186500 },
    { "Cygnus", 20.588000, 44.545000 },
    { "Delphinus", 20.693500, 11.671000 },
    { "Dorado", 5.241833, -59.387000 },
    { "Draco", 15.144000, 67.006667 },
    { "Equuleus", 21.187667, 7.758167 },
    { "Eridanus", 3.300333, -28.756167 },
    { "Fornax", 2.798000, -31.634500 },
    { "Gemini", 7.070667, 22.600167 },
    { "Grus", 22.456500, -46.351833 },
    { "Hercules", 17.386000, 27.498833 },
    { "Horologium", 3.276000, -53.336333 },
    { "Hydra", 11.612167, -14.531833 },
    { "Hydrus", 2.344167, -69.956500 },
    { "Indus", 21.972167, -59.706667 },
    { "Lacerta", 22.461333, 46.041833 },
    { "Leo", 10.667167, 13.138667 },
    { "Leo Minor", 10.245333, 32.134667 },
    { "Lepus", 5.565833, -19.046333 },
    { "Libra", 15.199333, -15.234667 },
    { "Lupus", 15.220167, -42.708833 },
    { "Lynx", 7.992167, 47.466667 },
    { "Lyra", 18.852833, 36.689333 },
    { "Mensa", 5.415000, -77.504000 },
    { "Microscopium", 20.964667, -36.274833 },
    { "Monoceros", 7.060500, 0.282167 },
    { "Musca", 12.588000, -70.161000 },
    { "Norma", 15.903000, -51.351500 },
    { "Octans", 23.000000, -82.152000 },
    { "Ophiuchus", 17.394833, -7.912333 },
    { "Orion", 5.576500, 5.949000 },
    { "Pavo", 19.611833, -65.781500 },
    { "Pegasus", 22.697333, 19.466333 },
    { "Perseus", 3.175000, 45.013167 },
    { "Phoenix", 0.931833, -48.580667 },
    { "Pictor", 5.707667, -53.474167 },
    { "Pisces", 0.482833, 13.687167 },
    { "Piscis Austrinus", 22.284500, -30.642167 },
    { "Puppis", 7.258000, -31.177333 },
    { "Pyxis", 8.952667, -27.351667 },
    { "Reticulum", 3.921167, -59.997500 },
    { "Sagitta", 19.650833, 18.861333 },
    { "Sagittarius", 19.099000, -28.476833 },
    { "Scorpius", 16.887333, -27.031500 },
    { "Sculptor", 0.438000, -32.088333 },
    { "Scutum", 18.673167, -9.888667 },
    { "Serpens (Caput)", 15.774833, 10.970000 },
    { "Serpens (Cauda)", 18.126667, -4.862167 },
    { "Sextans", 10.271500, -2.614667 },
    { "Taurus", 4.702167, 14.877167 },
    { "Telescopium", 19.325667, -51.036833 },
    { "Triangulum", 2.184500, 31.476000 },
    { "Triangulum Australe", 16.082500, -65.388000 },
    { "Tucana", 23.777333, -65.830000 },
    { "Ursa Major", 11.312667, 50.721167 },
    { "Ursa Minor", 15.000000, 77.699833 },
    { "Vela", 9.577333, -47.167167 },
    { "Virgo", 13.406500, -4.158500 },
    { "Volans", 7.795500, -69.801167 },
    { "Vulpecula", 20.231333, 24.442667 }
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
