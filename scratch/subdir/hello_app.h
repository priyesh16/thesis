// hello-app.h

#ifndef HELLO_APP_H_
#define HELLO_APP_H_

#include "ns3/ndn-app.h"

namespace ns3 {

class HelloApp : public ndn::App
{
public:
  // register NS-3 type "HelloApp"
  static TypeId
  GetTypeId ();
  
  // (overridden from ndn::App) Processing upon start of the application
  virtual void
  StartApplication ();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void
  StopApplication ();

  // (overridden from ndn::App) Callback that will be called when Interest arrives
  virtual void
  OnInterest (Ptr<const ndn::Interest> interest);

  // (overridden from ndn::App) Callback that will be called when Data arrives
  virtual void
  OnData (Ptr<const ndn::Data> contentObject);

private:
  void
  SendInterest ();
};

} // namespace ns3

#endif // HELLO_APP_H_