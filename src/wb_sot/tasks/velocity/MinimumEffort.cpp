/*
 * Copyright (C) 2014 Walkman
 * Author: Alessio Rocchi, Enrico Mingo
 * email:  alessio.rocchi@iit.it, enrico.mingo@iit.it
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU Lesser General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#include <wb_sot/tasks/velocity/MinimumEffort.h>
#include <drc_shared/cartesian_utils.h>
#include <exception>
#include <cmath>

using namespace wb_sot::tasks::velocity;
using namespace yarp::math;

MinimumEffort::MinimumEffort(   const yarp::sig::Vector& x) :
    Task("posture", x, x.size()), _gTauGradientWorker(x)
{
    _W.resize(_x_size, _x_size);
    _W.eye();

    for(unsigned int i = 0; i < _x_size; ++i)
        _W(i,i) = 1.0 / (_robot.coman_iDyn3.getJointTorqueMax()[i]
                                        *
                        _robot.coman_iDyn3.getJointTorqueMax()[i]);

    _gTauGradientWorker.setW(_W);

    _A.resize(_x_size, _x_size);
    _A.eye();

    /* first update. Setting desired pose equal to the actual pose */
    this->update(_x0);
}

MinimumEffort::~MinimumEffort()
{
   //_referenceInputPort.close();
}

void MinimumEffort::update(const yarp::sig::Vector &x) {

    _x = x;
    /************************* COMPUTING TASK *****************************/

    _b = -1.0 * cartesian_utils::computeGradient(_x, _gTauGradientWorker);

    /**********************************************************************/
}



