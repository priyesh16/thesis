/*
 * myndn.h
 *
 *  Created on: May 19, 2016
 *      Author: pri
 */

#ifndef SCRATCH_MYNDN_H_
#define SCRATCH_MYNDN_H_

#include <stdlib.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"

#include "../src/ndnSIM/utils/trie/trie.h"
#include "../src/ndnSIM/utils/trie/trie-with-policy.h"
#include "../src/ndnSIM/utils/trie/counting-policy.h"
#include "../src/ndnSIM/ndn.cxx/detail/pending-interests-container.h"
#include "../src/ndnSIM/ndn.cxx/detail/registered-prefix-container.h"
#include "../src/ndnSIM/helper/ndn-global-routing-helper.h"
#include "../src/ndnSIM/model/ndn-net-device-face.h"
#include "../src/ndnSIM/model/ndn-global-router.h"

using namespace std;
using namespace ns3;
using namespace ndn;

#define CONS 3 //node h
#define PROD 13 // node q
#define ANCHORSCNT 4
#define INVALID_PARENT_ID 0xbadbabe
#define NODE_CNT 153
//#define NODE_CNT 18

#define FILENAME "scratch/ATT_topo.txt"
//#define FILENAME "scratch/paper_topo.txt"

unsigned anchorsCnt = ANCHORSCNT;
unsigned prod = PROD;
ndn::Name initialPrefixName = Name("/initial");
ndn::Name invalidPrefixName = Name("/invalid");
ndn::Name rootPrefixName = Name("/0");
unsigned int rootId = 1000;// Assign an invalid root id in the begining

ndn::ndnSIM::trie_with_policy< ndn::Name,
									ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
									ndn::ndnSIM::counting_policy_traits > namesTrie;


typedef ndn::ndnSIM::trie_with_policy< ndn::Name,
					ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
					ndn::ndnSIM::counting_policy_traits > twoNbrTrie;


typedef ndn::ndnSIM::trie_with_policy< Name,
                                    ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
                                    ndnSIM::counting_policy_traits > super;

std::string interestPrefixstr = "/prefix";

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
	unsigned int nodeId;
	std::list<Ptr<Node> > oneHopNodeList; //List of one hop nbrs.
	std::list<Ptr<NdnNode> > oneHopList; // List of Nodeinfos of one hop nbrs, basically 2hop nbrs

	std::list<Ptr<NdnNode> > childrenList; //List of one hop nbrs.
	std::string nodeName;    // like hostname
	unsigned int ndnNodeId;
	ndn::Name prefixName;
	ndn::ndnSIM::trie_with_policy< ndn::Name,
										ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
										ndn::ndnSIM::counting_policy_traits > *nbrTrie;
	Ptr<Node> nextHopNode; //Next node to route to (This is to be deleted and directly added to fib)
	unsigned int parentId;
	Ptr<NdnNode> anchorNode;
	std::list<Ptr<NdnNode> > anchorChildrenList;
	//ndn::AppHelper *pHelloApp;
	// like ip address
// (*oneHopInfoList).oneHopList is the list of twoHopNbrs going through that oneHopNbr
// note the twoHopNbr could be the source node also..so always check for that
} ;

std::vector<NdnNode> ndnNodeContainer;
std::list<Ptr<NdnNode> > anchorList;

typedef enum err_s
{
	NDN_OK,
	NDN_PARENT_HAS_NO_NAME,
}err_t;

typedef enum direction_s
{
	NDN_INCREASING_NODE_ID,
	NDN_ROOT_TO_CHILDREN,
}direction_t;

typedef enum routetype_s
{
	DIJKSTRA,
	AIR,
}routetype_t;



//int ndnNodeIdTable[NODE_CNT];

int ndnNodeIdTable[NODE_CNT] = {
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

void
add_path(unsigned firstNode,unsigned SecndNode, int metric, const string str);

void fill_two_hop_nbr_info();

void OnData (Ptr<Face> pFace, Ptr<Data> data);

/*
void FindParentThruMsg(Ptr<Face> pFace, NdnPacket ndnPacket);

void AssignPrefixNameThruMsg(Ptr<Face> pFace, NdnPacket ndnPacket);
*/
void NotifyParentChange(unsigned int curId, unsigned int preId, unsigned int postId);

void NotifyNameChange(unsigned int curId, Name preName, Name postName);

Ptr<NdnNode> GetNdnNode(Ptr<Node> curNode);

void PrintChildren(Ptr<Node> curNode);

void FillChildrenList(Ptr<Node> curNode);

void FindParent(Ptr<Node> curNode);

typedef void (*AllCallFuncttion)(Ptr<Node> curNode);

void AllNodesCall(AllCallFuncttion function, direction_t direction);


unsigned int HashFunction(unsigned int curNodeId);

void IdentifyAnchors();

void PublishToAnchor(Ptr<Node> curNode);

Ptr<NdnNode> SubscribeToAnchor(Ptr<Node> curNode);

Ptr<NdnNode> GetNdnNodefromNode(Ptr<Node> curNode);

Ptr<NdnNode> GetNdnNodefromId(unsigned int ndnNodeId);

void FillTwoHopTrie(Ptr<Node> curNode);

void AddFibEntries (Ptr<Node> curNode) ;

void DeleteTree(Ptr<Node> curNode);

void CreateNDNNodeIdTable();

void CreateNodeContainer();

void FillOneHopNbrList(Ptr<Node> curNode);

void GetRootId();

void AssignPrefixName(Ptr<Node> curNode);

void FindNextHop(Ptr<Node> curNode);


#endif /* SCRATCH_MYNDN_H_ */
