// hello-app.cc

#include "hello_app.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"

#include "ns3/ndn-fib.h"
#include "ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE ("HelloApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (HelloApp);


// register NS-3 type
TypeId
HelloApp::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ndn::HelloApp")
    .SetParent<ndn::App> ()
    .AddConstructor<HelloApp> ()
    ;
  return tid;
}

// Processing upon start of the application
void
HelloApp::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication ();

  // Create a name components object for name ``/prefix/sub``
  Ptr<ndn::Name> prefix = Create<ndn::Name> (); // now prefix contains ``/``
  prefix->append ("prefix"); // now prefix contains ``/prefix``
  prefix->append ("sub"); // now prefix contains ``/prefix/sub``

  /////////////////////////////////////////////////////////////////////////////
  // Creating FIB entry that ensures that we will receive incoming Interests //
  /////////////////////////////////////////////////////////////////////////////

  // Get FIB object
  Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();

  // Add entry to FIB
  // Note that ``m_face`` is cretaed by ndn::App
  Ptr<ndn::fib::Entry> fibEntry = fib->Add (*prefix, m_face, 0);

  Simulator::Schedule (Seconds (1.0), &HelloApp::SendInterest, this);
  Simulator::Schedule (Seconds (1.0), &HelloApp::SendHello, this);
}

// Processing when application is stopped
void
HelloApp::StopApplication ()
{
  // cleanup ndn::App
  ndn::App::StopApplication ();
}

void
HelloApp::SendInterest ()
{
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////
  
  Ptr<ndn::Name> prefix = Create<ndn::Name> ("/prefix/sub"); // another way to create name

  // Create and configure ndn::Interest
  Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  interest->SetNonce            (rand.GetValue ());
  interest->SetName             (prefix);
  interest->SetInterestLifetime (Seconds (1.0));

  NS_LOG_DEBUG ("Sending Interest packet for " << *prefix);
  
  // Call trace (for logging purposes)
  m_transmittedInterests (interest, this, m_face);

  m_face->ReceiveInterest (interest);
}

Ptr<NdnNode>
HelloApp::GetNdnNode(Ptr<Node> curNode)
{
  int pos;
	int topoId;
  
  topoId = curNode->GetId();
	pos = nodeIdTable[topoId];
	
  return &ndnNodeContainer[pos];
}

Ptr<Face> 
HelloApp::GetFace(unsigned firstNodeId, unsigned secondNodeId) 
{
	Ptr<Node> node1 = NodeList::GetNode(firstNodeId);
	Ptr<Channel> channel;
	Ptr<ndn::L3Protocol> l3Prot;
	Ptr<Face> face;

	unsigned i, j;
	for(i = 0; i < node1->GetNDevices(); i++) {
		channel = node1->GetDevice(i)->GetChannel();
		for(j = 0; j < channel->GetNDevices(); j++){
			if(channel->GetDevice(j)->GetNode()->GetId() == secondNodeId){
				//Ptr<Ipv4> stack = node->GetObject<Ipv4> ();
				l3Prot = node1->GetObject<ndn::L3Protocol> ();
		       	face = l3Prot->GetFace(i);
			}
		}
	}
	return face;
}

void 
HelloApp::SendHello()
{
	NdnPacket ndnPacket;
	std::list<Ptr<Node> > oneHopList;
	std::list<Ptr<Node> >::iterator oneHopListIter;
	//unsigned int size = oneHopList.size();
  Ptr<Node> curNode = GetNode ();
	Ptr<NdnNode> curNdnNode = GetNdnNode(curNode);
	Ptr<Node> nbrNode;
	Ptr<Face> face;
	Data data;
	
	ndnPacket.packetType = GET_LABEL;
	ndnPacket.senderId = curNdnNode->nodeId;
	ndnPacket.rootId = curNdnNode->rootId; 
	Packet payload((uint8_t *)&ndnPacket, sizeof(NdnPacket));
	data.SetPayload(&payload);
	
	//send packet to all its neighbours;
	for(oneHopListIter = oneHopList.begin() ; oneHopListIter != oneHopList.end() ; oneHopListIter++ ) {
		nbrNode = *oneHopListIter;
		face = GetFace(curNode->GetId(), nbrNode->GetId());
		face->SendData(&data);
	}

	cout << "Packet sent from %d" << curNdnNode->nodeName << "\n";
}



// Callback that will be called when Interest arrives
void
HelloApp::OnInterest (Ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest (interest);
  
  NS_LOG_DEBUG ("Received Interest packet for " << interest->GetName ());

  // Create and configure ndn::Data and ndn::DataTail
  // (header is added in front of the packet, tail is added at the end of the packet)

  // Note that Interests send out by the app will not be sent back to the app !
  
  Ptr<ndn::Data> data = Create<ndn::Data> (Create<Packet> (1024));
  data->SetName (Create<ndn::Name> (interest->GetName ())); // data will have the same name as Interests

  NS_LOG_DEBUG ("Sending Data packet for " << data->GetName ());  

  // Call trace (for logging purposes)
  m_transmittedDatas (data, this, m_face);

  m_face->ReceiveData (data); 
}

// Callback that will be called when Data arrives
void
HelloApp::OnData (Ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG ("Receiving Data packet for " << data->GetName ());
  Ptr <const Packet> packet;
  uint8_t buffer[sizeof(NdnPacket)];
  Ptr <NdnPacket> ndnPacket = (NdnPacket *)buffer;
  	
  packet = data->GetPayload();
  packet->CopyData(buffer, sizeof(NdnPacket));

  cout << "Root is " << ndnPacket->rootId << "\n";

  std::cout << "DATA received for name " << data->GetName () << std::endl;
}

} // namespace ns3