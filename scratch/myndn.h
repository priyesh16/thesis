/*
 * myndn.h
 *
 *  Created on: May 19, 2016
 *      Author: pri
 */

#ifndef SCRATCH_MYNDN_H_
#define SCRATCH_MYNDN_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"
#include "../src/ndnSIM/utils/trie/trie.h"
#include "../src/ndnSIM/utils/trie/trie-with-policy.h"
#include "../src/ndnSIM/utils/trie/counting-policy.h"
#include "../src/ndnSIM/ndn.cxx/detail/pending-interests-container.h"
#include "../src/ndnSIM/ndn.cxx/detail/registered-prefix-container.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"
#include "../src/ndnSIM/helper/ndn-global-routing-helper.h"
#include "../src/ndnSIM/model/ndn-net-device-face.h"
#include "../src/ndnSIM/model/ndn-global-router.h"

using namespace std;
using namespace ns3;
using namespace ndn;

typedef ndn::ndnSIM::trie_with_policy< ndn::Name,
					ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
					ndn::ndnSIM::counting_policy_traits > twoNbrTrie;


typedef ndn::ndnSIM::trie_with_policy< Name,
                                    ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
                                    ndnSIM::counting_policy_traits > super;

std::string interestPrefixstr = "/prefix";

class NodeInfo {
public:
	Ptr<Node> node;
	std::list<Ptr<Node> > oneHopList; //List of one hop nbrs.
	std::list<NodeInfo *> oneHopNodeInfoList; // List of Nodeinfos of one hop nbrs.
	std::string nodeName;    // like hostname
	std::string prefixStr;
	int nodeId;
	Ptr<ndn::Name> prefixName;
	twoNbrTrie *nbrTrie;
	Ptr<Node> nextHopNode; //Next node to route to (This is to be deleted and directly added to fib)
	bool isRoot; // Member to check if currently is root.
	// like ip address
// (*oneHopInfoList).oneHopList is the list of twoHopNbrs going through that oneHopNbr
// note the twoHopNbr could be the source node also..so always check for that
} ;

typedef enum packetType_s {
	GET_PARENT,
	GET_PREFIXNAME,
}packetType_t;

class NdnNode : public SimpleRefCount<NdnNode>
{
public:
	NdnNode(void){};	
	~NdnNode(void) {this->oneHopList.clear();};	

	Ptr<Node> pNode;
	std::list<Ptr<Node> > oneHopList; //List of one hop nbrs.
	std::list<NodeInfo *> oneHopNodeInfoList; // List of Nodeinfos of one hop nbrs, basically 2hop nbrs
	std::string nodeName;    // like hostname
	std::string prefixStr;
	int ndnNodeId;
	Ptr<ndn::Name> prefixName;
	twoNbrTrie *nbrTrie;
	Ptr<Node> nextHopNode; //Next node to route to (This is to be deleted and directly added to fib)
	int parentId;
	//ndn::AppHelper *pHelloApp; 
	// like ip address
// (*oneHopInfoList).oneHopList is the list of twoHopNbrs going through that oneHopNbr
// note the twoHopNbr could be the source node also..so always check for that
} ;

std::vector<NdnNode> ndnNodeContainer;

class NdnPacket : public SimpleRefCount<NdnPacket>
{
public:
	packetType_t packetType;
	int senderId;
	int receiverId;
	int parentId; //Node identifier which the sender thinks is the root
};


int ndnNodeIdTable[] = {
		7,		//a 0
		3, 		//b 1
		6,		//c 2
		13,		//d 3
		8,		//e 4
		2,		//f 5
		4,		//g 6
		5,		//h 7
		1,		//i 8
		0,		//j 9
		12,		//k 10
		14,		//m 11
		10,		//n 12
		9,		//o 13
		16,		//p 14
		15,		//q 15
		11,		//r 16
		17 		//s 17
};

std::string prefixNamesArr[] = {
		"/0/2/1",		//a
		"/0/1/1/1", 	//b
		"/0/2",			//c
		"/0/3/1",		//d
		"/0/2/1/1",		//e
		"/0/1/1",		//f
		"/0/1/1/1/1",	//g
		"/0/1/1/1/2",	//h
		"/0/1",			//i
		"/0",			//j
		"/0/3",			//k
		"/0/3/1/1",		//m
		"/0/2/1/1/2",	//n
		"/0/2/1/1/1",	//o
		"/0/3/1/2",		//p
		"/0/3/1/1/1",	//q
		"/0/2/1/1/2/1",	//r
		"/0/3/1/2/1" // s
};

#define NODE_CNT 18
#define CONS 7 //node h
#define PROD 15 // node q
#define DEST PROD

void
add_path(unsigned firstNode,unsigned SecndNode, int metric, const string str);

void fill_two_hop_nbr_info();

void OnData (Ptr<Face> pFace, Ptr<Data> data);

void FindParent(Ptr<Face> pFace, NdnPacket ndnPacket);

void AssignPrefixName(Ptr<Face> pFace, NdnPacket ndnPacket) ;

Ptr<NdnNode>
GetNdnNode(Ptr<Node> curNode);

#endif /* SCRATCH_MYNDN_H_ */
