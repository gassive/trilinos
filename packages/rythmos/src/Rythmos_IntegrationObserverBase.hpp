
#ifndef RYTHMOS_INTEGRATION_OBSERVER_BASE_HPP
#define RYTHMOS_INTEGRATION_OBSERVER_BASE_HPP

#include "Rythmos_Types.hpp"
#include "Teuchos_Describable.hpp"
#include "Teuchos_VerboseObject.hpp"

namespace Rythmos {


// Forwards
template<class Scalar> class TimeRange;
template<class Scalar> class StepControlInfo;
template<class Scalar> class StepperBase;


/** \brief Base class for strategy objects that observe and time integration
 * by observing the stepper object.
 *
 * ToDo: Finish Implementation!
 */
template<class Scalar>
class IntegrationObserverBase
  : virtual public Teuchos::Describable,
    virtual public Teuchos::VerboseObject<IntegrationObserverBase<Scalar> >
{
public:

  /** \brief Clone this integration observer if supported .
   *
   * Here, the cloned object just has to have the ouptut strategy copied, not
   * the complete state of the object mid way through an integration.
   */
  virtual RCP<IntegrationObserverBase<Scalar> >
  cloneIntegrationObserver() const = 0;

  /** \brief Reset the observer to prepair it to observe another integration.
   *
   * \param integrationTimeDomain [in] The time domain over which the
   * integration will be defined.
   *
   * <b>Preconditions:</b><ul>
   * <li> <tt>integrationTimeDomain.length() > 0.0</tt>
   * </ul>
   */
  virtual void resetIntegrationObserver(
    const TimeRange<Scalar> &integrationTimeDomain
    ) = 0;

  /** \brief Observer an integration step.
   *
   * \param stepper [in] The stepper object that was just stepped forward once
   * to integrate the transient ODE/DAE equations.  On the very first call and
   * every other call, this stepper should have a finite time range for a
   * successfull step..
   *
   * \param stepCtrlInfo [in] The info for the actual time step that was just
   * completed.
   *
   * \param timeStepIter [in] The time step iteration counter.  In the first
   * call to this function, this should be <tt>timeStepIter==0</tt> and it
   * should be incremented on each call only once.  While the concrete
   * implementation of <tt>*this</tt> could keep track of the this counter,
   * putting it in the argument list helps to simplify logic and helps to
   * validate correct usage.
   *
   * Warning! This function is *NOT* stateless.  It should be called once and
   * only once per time step iteration.
   *
   * NOTE: The function <tt>resetIntegrationControlStrategy()</tt> must be
   * called prior to even the first call to function.
   */
  virtual void observeCompletedTimeStep(
    const StepperBase<Scalar> &stepper,
    const StepControlInfo<Scalar> &stepCtrlInfo,
    const int timeStepIter
    ) = 0;

};


} // namespace Rythmos


#endif // RYTHMOS_INTEGRATION_OBSERVER_BASE_HPP
