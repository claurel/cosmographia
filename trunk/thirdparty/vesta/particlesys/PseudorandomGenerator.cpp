/*
 * $Revision$ $Date$
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "PseudorandomGenerator.h"
#include <iostream>

using namespace vesta;
using namespace std;


int main(void)
{
    PseudorandomGenerator gen;

    cout << "unsigned:\n";
    for (int i = 0; i < 20; ++i)
    {
        cout << gen.randFloat() << endl;
    }

    cout << "signed:\n";
    for (int i = 0; i < 20; ++i)
    {
        cout << gen.randSignedFloat() << endl;
    }

    return 0;
}

