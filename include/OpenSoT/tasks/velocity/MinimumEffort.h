
/*
 * Copyright (C) 2014 Walkman
 * Authors:Alessio Rocchi, Enrico Mingo
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

#ifndef __TASKS_VELOCITY_MINIMUMEFFORT_H__
#define __TASKS_VELOCITY_MINIMUMEFFORT_H__

 #include <OpenSoT/Task.h>
 #include <drc_shared/idynutils.h>
 #include <drc_shared/cartesian_utils.h>
 #include <yarp/sig/all.h>
 #include <yarp/math/Math.h>

 using namespace yarp::math;

 namespace OpenSoT {
    namespace tasks {
        namespace velocity {
            /**
             * @brief The MinimumEffort class implements a task that tries to bring the robot in a minimum-effort posture.
             * Notice that the weight of the robot is not taken into account when computing effort on the legs
             * (that is, the forces on the floating base are not projected on the contact points jacobians).
             * Also, the minimum effort task is using a simple gradient worker, ComputeGTauGradient, which does not satisfy contact points constraints
             * while performing the configuration vector needed to numerically compute the gradient. In particular, the gravity vector
             * is computed considering a support foot always in contact with the ground.
             * This means in general the minimum effort task should be used together with a cartesian task on the swing foot, implemented
             * through the OpenSoT::tasks::velocity::Cartesian class.
             * @code
             *
             *    iDynUtils robot;
             *    yarp::sig Vector q(nJ,0.0),dq(nJ,0.0);
             *    taskCartesianRSole = boost::shared_ptr<OpenSoT::tasks::velocity::Cartesian>(
             *                            new OpenSoT::tasks::velocity::Cartesian("cartesian::r_sole",q,robot,
             *                                                                    robot.right_leg.end_effector_name,
             *                                                                    robot.left_leg.end_effector_name));
             *    swing_foot_pos_ref = taskCartesianRSole->getReference();
             *
             * /// JOINT SPACE TASKS
             *    taskMinimumEffort = boost::shared_ptr<OpenSoT::tasks::velocity::MinimumEffort>(
             *        new OpenSoT::tasks::velocity::MinimumEffort(q));
             *
             * /// SOT
             *    stack_of_tasks.push_back(taskCartesianRSole);
             *    stack_of_tasks.push_back(taskMinimumEffort);
             *
             *    qpOasesSolver = OpenSoT::solvers::QPOases_sot::SolverPtr(
             *            new OpenSoT::solvers::QPOases_sot(stack_of_tasks));
             *    control_computed = qpOasesSolver->solve(dq_ref);
             *
             *
             * @endcode
             */
            class MinimumEffort : public Task < yarp::sig::Matrix, yarp::sig::Vector > {
            protected:
                iDynUtils _robot;
                yarp::sig::Vector _x;

                /**
                 * @brief The ComputeGTauGradient class implements a worker class to computes the effort for a certain configuration.
                 * It will take into account the robot as standing on a flat floor, and while computing the gradient,
                 * we will assume a foot is on the ground. Notice that the way we compute the gradient does not satisfy the constraints
                 * of both flat foot on the ground. So in general this simple implementation of the gradient worker needs to be used
                 * in a minimum effort task together with a constraint (or a higher priority task) for the swing foot.
                 */
                class ComputeGTauGradient : public cartesian_utils::CostFunction {
                    public:
                    iDynUtils _robot;
                    yarp::sig::Matrix _W;
                    ComputeGTauGradient(const yarp::sig::Vector& q) :
                        _W(q.size(),q.size())
                    { _W.eye(); this->update(q); }
                    double compute(const yarp::sig::Vector &q) {
                        _robot.updateiDyn3Model(q, true);
                        yarp::sig::Vector tau = _robot.coman_iDyn3.getTorques();
                        return yarp::math::dot(tau, _W * tau);
                    }

                    void setW(const yarp::sig::Matrix& W) { _W = W; }
                };

                ComputeGTauGradient _gTauGradientWorker;

            public:

                MinimumEffort(const yarp::sig::Vector& x);

                ~MinimumEffort();

                /**
                 * @brief _update updates the minimum effort gradient.
                 * @detail It also updates the state on the internal robot model so that successive calls to the
                 * computeEffort() function will take into account the updated posture of the robot.
                 * @param x the actual posture of the robot
                 */
                void _update(const yarp::sig::Vector& x);

                /**
                 * @brief computeEffort
                 * @return the effort at the actual configuration q (ast from latest update(q))
                 */
                double computeEffort();
            };
        }
    }
 }

#endif
