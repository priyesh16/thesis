/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */
// ndn-simple.cc

#include "ndn_air.h"


/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-simple
 */




Ptr<Node> from;
Ptr<Node> to;

std::vector<std::string> names;
ns3::AnnotatedTopologyReader topologyReader("", 1);
//std::vector<NodeInfo> nbrTable(NODE_CNT);


//NodeContainer nodeContainer;

ndn::ndnSIM::trie_with_policy< ndn::Name,
					ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
					ndn::ndnSIM::counting_policy_traits> prefixTrie;

std::vector<Ptr<ndn::Name> > prefix;

void CreateNodeContainer() {
	std::vector<std::string>::const_iterator namesIter;
	NodeContainer nodeContainer = NodeContainer::GetGlobal();
	std::list<TopologyReader::Link> links;
	links = topologyReader.GetLinks();
	std::list<TopologyReader::Link>::iterator linkiter;
	std::string fromName = "";
	std::string toName = "";
	unsigned int pos;
	unsigned int topoId;
	unsigned int size = nodeContainer.GetN();
	ndnNodeContainer.resize(size);
	ndn::Name initialPrefixName = Name("/initial");

	for(linkiter = links.begin() ; linkiter != links.end() ; linkiter++ ) {

		from = (*linkiter).GetFromNode();
		fromName = (*linkiter).GetFromNodeName();
		to = (*linkiter).GetToNode();
		toName = (*linkiter).GetToNodeName();

		//std::cout << "Pri : " << fromName << "\t\t -> " << toName << " : " << to->GetId() << " " << pos << "\n";
		topoId = from->GetId();
		pos = ndnNodeIdTable[topoId];
		ndnNodeContainer[pos].pNode = from;
		ndnNodeContainer[pos].nodeId = topoId;
		ndnNodeContainer[pos].nodeName = fromName;
		ndnNodeContainer[pos].oneHopNodeList.push_back(to);
		ndnNodeContainer[pos].ndnNodeId = ndnNodeIdTable[topoId];
		ndnNodeContainer[pos].parentId = INVALID_PARENT_ID;
		ndnNodeContainer[pos].prefixName = initialPrefixName;
		ndnNodeContainer[pos].nbrTrie = new ndn::ndnSIM::trie_with_policy< ndn::Name,
										ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
										ndn::ndnSIM::counting_policy_traits > (1);
		//ndnNodeContainer[pos].pHelloApp = NULL;

		//std::cout << "Pri : " << toName << "\t\t -> " << fromName << " : " << from->GetId() << " " << pos << "\n";
		topoId = to->GetId();
		pos = ndnNodeIdTable[topoId];
		ndnNodeContainer[pos].pNode = to;
		ndnNodeContainer[pos].nodeId = topoId;
		ndnNodeContainer[pos].nodeName = toName;
		ndnNodeContainer[pos].oneHopNodeList.push_back(from);
		ndnNodeContainer[pos].ndnNodeId = ndnNodeIdTable[topoId];
		ndnNodeContainer[pos].parentId = INVALID_PARENT_ID;
		ndnNodeContainer[pos].prefixName = initialPrefixName;
		ndnNodeContainer[pos].nbrTrie = new ndn::ndnSIM::trie_with_policy< ndn::Name,
										ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
										ndn::ndnSIM::counting_policy_traits > (1);

		//ndnNodeContainer[pos].pHelloApp = NULL;
	}
}



void FillOneHopNbrList(Ptr<Node> curNode)
{
	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
	std::list<Ptr<Node> >::iterator oneHopListIter;
  	Ptr<Node> nbrNode;

	std::list<Ptr<Node> > oneHopNodeList = curNdnNode->oneHopNodeList;

	for(oneHopListIter = oneHopNodeList.begin() ; oneHopListIter != oneHopNodeList.end() ; oneHopListIter++ ) {
		nbrNode = *(oneHopListIter);
		curNdnNode->oneHopList.push_back(GetNdnNodefromNode(nbrNode));
	}

}

Ptr<Face> GetFace(unsigned firstNodeId, unsigned secondNodeId)
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

Ptr<NdnNode> GetNdnNodefromNode(Ptr<Node> curNode)
{
	unsigned int pos;
	unsigned int topoId;

	topoId = curNode->GetId();
	pos = ndnNodeIdTable[topoId];

	return &ndnNodeContainer[pos];
}

Ptr<NdnNode> GetNdnNodefromId(unsigned int ndnNodeId)
{

	unsigned int i;
	unsigned int flag = 0;

	for (i = 0; i < NODE_CNT; i++) {
		if (ndnNodeContainer[i].ndnNodeId == ndnNodeId) {
			flag = 1;
			break;
		}
	}

	if (flag == 0){
		cout << "Couldn't find id \n";
	}
	return &(ndnNodeContainer[i]);

}


unsigned int GetChildId(Ptr<NdnNode> parentNdnNode, Ptr<NdnNode> curChildNdnNode)
{
	std::list<Ptr<NdnNode> >::iterator childrenListIter;
	Ptr<NdnNode> childNdnNode;
	Ptr<Face> pFace;
	unsigned int childId = 0;

	std::list<Ptr<NdnNode> > childrenList = parentNdnNode->childrenList;

	for(childrenListIter = childrenList.begin() ; childrenListIter != childrenList.end() ; childrenListIter++ ) {
		childNdnNode = *(childrenListIter);
		if (curChildNdnNode->ndnNodeId == childNdnNode->ndnNodeId) {
			break;
		}
		childId++;
	}
	return childId;
}


void NotifyNameChange(unsigned int curId, Name preName, Name postName)
{
	Ptr<NdnNode> curNdnNode = GetNdnNodefromId(curId);
	string curName = curNdnNode->nodeName;

	cout << "<!!Notification!!> I am " << curName << "(id:" << curId <<  ")";
	cout << " and I'm changing my name from " << preName.toUri();
	cout << " to " << postName.toUri() << "\n";
}

void NotifyParentChange(unsigned int curId, unsigned int preId, int postId)
{
	Ptr<NdnNode> curNdnNode = GetNdnNodefromId(curId);
	Ptr<NdnNode> preNdnNode = GetNdnNodefromId(preId);
	Ptr<NdnNode> postNdnNode = GetNdnNodefromId(postId);
	string curName = curNdnNode->nodeName;
	string preName = preNdnNode->nodeName;
	string postName = postNdnNode->nodeName;

	cout << "<!!Notification!!> I am " << curName << "(id:" << curId <<  ")";
	cout << " and I'm changing my parent from " << preName << "(id:" << preId <<  ")";
	cout << " to " << postName << "(id:" << postId <<  ")" <<  "\n";
}


void AssignPrefixName(Ptr<Node> curNode)
{
	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
	Ptr<NdnNode> childNdnNode;
	unsigned int childId;
	string strChildId;
	ndn::Name preName;
	std::list<Ptr<NdnNode> >::iterator childrenListIter;
  	Name tmpPrefixName;
	std::list<Ptr<NdnNode> > childrenList = curNdnNode->childrenList;

	for(childrenListIter = childrenList.begin() ; childrenListIter != childrenList.end() ; childrenListIter++ ) {
		std::stringstream sstream;
		tmpPrefixName = Name(curNdnNode->prefixName.toUri());
		childNdnNode = *(childrenListIter);
		preName = childNdnNode->prefixName;
		childId = GetChildId(curNdnNode, childNdnNode);
		sstream << childId;
		strChildId = sstream.str();
		childNdnNode->prefixName = tmpPrefixName.append(strChildId);
		NotifyNameChange(childNdnNode->ndnNodeId, preName, childNdnNode->prefixName);
	}

	return;
}


void PrintChildren(Ptr<Node> curNode)
{
	std::list<Ptr<NdnNode> >::iterator childrenListIter;
	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
  	Ptr<NdnNode> childNdnNode;

	std::list<Ptr<NdnNode> > childrenList = curNdnNode->childrenList;

	cout << "I am " << curNdnNode->nodeName << "(id:" << curNdnNode->ndnNodeId <<  ")";
	cout << " and my children are :\n";
	//send packet to all its neighbours;
	for(childrenListIter = childrenList.begin() ; childrenListIter != childrenList.end() ; childrenListIter++ ) {
		childNdnNode = *(childrenListIter);
		cout << "\t\t\t\t\t" << childNdnNode->nodeName << "(id:" << childNdnNode->ndnNodeId <<  ")\n";
	}
}

void FillChildrenList(Ptr<Node> curNode)
{
	std::list<Ptr<NdnNode> >::iterator oneHopListIter;
	//unsigned int size = oneHopList.size();
  	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
  	Ptr<NdnNode> nbrNdnNode;

	std::list<Ptr<NdnNode> > oneHopList = curNdnNode->oneHopList;

	for(oneHopListIter = oneHopList.begin() ; oneHopListIter != oneHopList.end() ; oneHopListIter++ ) {
		nbrNdnNode = *(oneHopListIter);

		// If neighbour already has a parent then it is not a child
		if (curNdnNode->ndnNodeId != rootId &&
			nbrNdnNode->parentId != INVALID_PARENT_ID) {
				continue;
		}

		nbrNdnNode->parentId = curNdnNode->ndnNodeId;
		curNdnNode->childrenList.push_back(nbrNdnNode);
	}
}

void GetRootId()
{
	Ptr<NdnNode> curNdnNode;
	Ptr<Node> curNode;
	NodeContainer nodeContainer = NodeContainer::GetGlobal();
	unsigned int size = ndnNodeContainer.size();
	for (unsigned int i = 0; i < size; i++) {
		curNode = nodeContainer.Get (i);
		curNdnNode = GetNdnNodefromNode(curNode);
		if (rootId > curNdnNode->ndnNodeId) {
			rootId = curNdnNode->ndnNodeId;
		}
	}
	curNdnNode = GetNdnNodefromId(rootId);
	std::list<Ptr<NdnNode> >::iterator oneHopListIter;
	//unsigned int size = oneHopList.size();
  	Ptr<NdnNode> nbrNdnNode;

	std::list<Ptr<NdnNode> > oneHopList = curNdnNode->oneHopList;

	//assign all neighbours as children
	for(oneHopListIter = oneHopList.begin() ; oneHopListIter != oneHopList.end() ; oneHopListIter++ ) {

		nbrNdnNode = *(oneHopListIter);
		nbrNdnNode->parentId = rootId;
		//curNdnNode->childrenList.push_back(nbrNdnNode);
	}
	// Assign roots parent to itself
	curNdnNode->parentId = rootId;
	curNdnNode->prefixName = Name("/0");

}



void PrintNeighbours(Ptr<Node> curNode)
{
	std::list<Ptr<NdnNode> >::iterator oneHopListIter;
	//unsigned int size = oneHopList.size();
  	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
  	Ptr<NdnNode> nbrNdnNode;

	std::list<Ptr<NdnNode> > oneHopList = curNdnNode->oneHopList;

	cout << "I am " << curNdnNode->nodeName << "(id:" << curNdnNode->ndnNodeId <<  ")";
	cout << " and my neighbours are :\n";
	for(oneHopListIter = oneHopList.begin() ; oneHopListIter != oneHopList.end() ; oneHopListIter++ ) {

		nbrNdnNode = *(oneHopListIter);

		cout << "\t\t\t\t\t" << nbrNdnNode->nodeName << "(id:" << nbrNdnNode->ndnNodeId <<  ")\n";
	}
}

void AllNodesCall(AllCallFuncttion function, direction_t direction)
{
	NodeContainer nodeContainer = NodeContainer::GetGlobal();
	unsigned int size = ndnNodeContainer.size();
	Ptr<NdnNode> rootNdnNode;
	std::list<Ptr<NdnNode> >::iterator childrenListIter;
	Ptr<NdnNode> childNdnNode;
	unsigned int curNodeId = 0;
	queue <Ptr<NdnNode> > q;
	unsigned int i;

	switch (direction) {
		case NDN_INCREASING_NODE_ID:
		{
			for (i = 0; i < size; i++) {
				function(nodeContainer.Get (i));
			}
			break;
		}
		case NDN_ROOT_TO_CHILDREN:
		{
			rootNdnNode = GetNdnNodefromId(rootId);
			q.push(rootNdnNode);
			while (q.empty() == false)
			{
 				Ptr<NdnNode> tmpNdnNode = q.front();
				curNodeId = tmpNdnNode->pNode->GetId();
				function(nodeContainer.Get (curNodeId));
				std::list<Ptr<NdnNode> > childrenList = tmpNdnNode->childrenList;
				for(childrenListIter = childrenList.begin() ; childrenListIter != childrenList.end() ; childrenListIter++ ) {
					q.push(*(childrenListIter));
				}
				q.pop();
			}
			break;
		}
	}
}



void PublishToAnchor(Ptr<Node> curNode)
{
	cout << "a-1\n";

	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
	unsigned int curNodeId = curNdnNode->ndnNodeId;
	unsigned int anchorId;
	Ptr<NdnNode> anchorNdnNode;

	cout << "a0\n";

	anchorId = HashFunction(curNodeId);
cout << "a1\n";
	anchorNdnNode = GetNdnNodefromId(anchorId);
cout << "a2\n";
	anchorNdnNode->anchorChildrenList.push_back(curNdnNode);
  cout << "a3\n";
}

unsigned int GetNdnIdfromNodeId(unsigned int nodeId)
{
	unsigned int i;
	unsigned int flag = 0;

	for (i = 0; i < NODE_CNT; i++) {
		if (ndnNodeContainer[i].nodeId == nodeId) {
			flag = 1;
			break;
		}
	}

	if (flag == 0){
		cout << "Couldn't find id " << nodeId << "\n";
		return -1;
	}
	return ndnNodeContainer[i].ndnNodeId;
}

Ptr<NdnNode> SubscribeToAnchor(Ptr<Node> curNode)
{
	unsigned int ndnNodeId = GetNdnIdfromNodeId(curNode->GetId());
	Ptr<NdnNode> anchorNdnNode;
	Ptr<NdnNode> curNdnNode;
	int found = 0;
	unsigned int anchorId;
	std::list<Ptr<NdnNode> >::iterator anchorListIter;

	anchorId = HashFunction(ndnNodeId);
	anchorNdnNode = GetNdnNodefromId(anchorId);

	std::list<Ptr<NdnNode> > anchorList = anchorNdnNode->anchorChildrenList;

	for(anchorListIter = anchorList.begin() ; anchorListIter != anchorList.end() ; anchorListIter++ ) {
		curNdnNode = *(anchorListIter);

		// If neighbour already has a parent then it is not a child
		if (curNdnNode->nodeId == curNode->GetId()) {
			found = 1;
			break;
		}
	}

	if (found == 0) {
		cout << "Couldn't find ndn node with node id " << curNode->GetId() << "\n";
		return 0;
	}
	return curNdnNode;
}

void FillTwoHopTrie(Ptr<Node> curNode)
{
	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
	std::list<Ptr<NdnNode> > oneHopList;
	std::list<Ptr<NdnNode> > twoHopList;
	std::list<Ptr<NdnNode> >::const_iterator oneHopListIter;
	std::list<Ptr<NdnNode> >::const_iterator twoHopListIter;
	Ptr<NdnNode> oneHopNbr;
	Name curPrefix = curNdnNode->prefixName;
	Name oneHopNbrPrefix;
	Name twoHopNbrPrefix;
	std::string oneHopNbrName;
	std::string twoHopNbrName;


	ndn::ndnSIM::trie_with_policy< ndn::Name,
										ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
										ndn::ndnSIM::counting_policy_traits > tmpTrie;

	oneHopList = curNdnNode->oneHopList;

	cout << "I am " << curNdnNode->nodeName << " : " << curPrefix.toUri() << "\n";

	for(oneHopListIter = oneHopList.begin() ; oneHopListIter != oneHopList.end() ; oneHopListIter++ ) {
		oneHopNbr = (*oneHopListIter);
		oneHopNbrName = (*oneHopListIter)->nodeName;
		oneHopNbrPrefix = (*oneHopListIter)->prefixName;
		cout << "\t" <<"1HopNbr " << oneHopNbrName << " : " << oneHopNbrPrefix.toUri() << "\n";
		twoHopList = (*oneHopListIter)->oneHopList;

		for (twoHopListIter = twoHopList.begin(); twoHopListIter != twoHopList.end(); twoHopListIter++) {
			twoHopNbrPrefix = (*twoHopListIter)->prefixName;
			twoHopNbrName = (*twoHopListIter)->nodeName;
			// If two hop nbr is the source then continue
			if(curPrefix == twoHopNbrPrefix) {
				//cout << "\t\tThe two hop Nbr " << twoHopNbrStr << " and the source are the same \n";
				continue;
			}

			cout << "\t\t" << "2HopNbr " << twoHopNbrName << " : " << twoHopNbrPrefix.toUri() << "\n";
			(*(curNdnNode->nbrTrie)).insert((twoHopNbrPrefix), Create < ndn::detail::RegisteredPrefixEntry > (&twoHopNbrPrefix));
			//*(curNdnNode->nbrTrie) = tmpTrie;
			//std::cout << tmpTrie.getTrie ();
		}
	}

}



void FindNextHop(Ptr<Node> curNode) {
	std::list<Ptr<NdnNode> > oneHopList;
	std::list<Ptr<NdnNode> > oneHopList1;
	std::list<Ptr<NdnNode> >::const_iterator oneHopInfoListIter;
	std::list<Ptr<NdnNode> >::const_iterator oneHopInfoListIter1;
	std::list<Ptr<NdnNode> >::reverse_iterator reviter;
	std::list<Ptr<NdnNode> >::const_iterator oneHopListIter;
	std::list<Ptr<NdnNode> > twoHopList;
	std::list<Ptr<NdnNode> > twoHopList1;
	std::list<Ptr<NdnNode> >::const_iterator twoHopListIter;
	std::list<Ptr<NdnNode> >::const_iterator twoHopListIter1;
	Ptr<Node> oneHopNbr;
	Ptr<Node> twoHopNbr;
	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
	NodeContainer nodeContainer = NodeContainer::GetGlobal();
	std::string prefixStr;
	std::string sourceName;
	std::string oneHopNbrName;
	std::string oneHopNbrPrefix;
	std::string twoHopNbrStr;
	std::string twoHopNbrPreStr;
	ndn::Name foundPrefStr;
	Ptr<ndn::Name> twoHopNbrName;
	std::vector<std::string>::const_iterator namesIter;
	std::vector<Ptr<ndn::Name> > tmpPrefix;
	super::iterator item;
	Ptr<NdnNode> dstNdnNode = SubscribeToAnchor(nodeContainer.Get(prod));
	std::string destPrefix = dstNdnNode->prefixName.toUri();
	Ptr<ndn::Name> destPrefixName = &(dstNdnNode->prefixName);
	Ptr<Node> nextHop;
	unsigned int found = 0;
	Ptr<ndn::Name> srcPrefixName;
	cout << "\n-------------------------------------------------\n";

	//curNdnNode = &ndnNodeContainer[i];
	sourceName = curNdnNode->nodeName;
	srcPrefixName = &(curNdnNode->prefixName);
	prefixStr = curNdnNode->prefixName.toUri();
	cout << "Sourcename " << sourceName << "\n";

	oneHopList = curNdnNode->oneHopList;
	// If source is the dest then break
	if(prefixStr == destPrefix) {
		cout << "Current node " << sourceName << " is the dest \n";
		found = 1;
		nextHop = 0;
		curNdnNode->nextHopNode = curNdnNode->pNode;
		return;
	}

	ndn::ndnSIM::trie_with_policy< ndn::Name,
										ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
										ndn::ndnSIM::counting_policy_traits > tmpTrie;

	for(oneHopInfoListIter = oneHopList.begin() ; oneHopInfoListIter != oneHopList.end() ; oneHopInfoListIter++ ) {
		oneHopNbr = (*oneHopInfoListIter)->pNode;
		oneHopNbrName = (*oneHopInfoListIter)->nodeName;
		oneHopNbrPrefix = (*oneHopInfoListIter)->prefixName.toUri();
		cout << "\t" <<"1HopNbr " << oneHopNbrName << "\n";
		twoHopList = (*oneHopInfoListIter)->oneHopList;
		// If one hop nbr is the dest then break
		if(oneHopNbrPrefix == destPrefix) {
			cout << "Next hop is " << oneHopNbrName << " (which is also the dest) \n";
			found = 1;
			nextHop = oneHopNbr;
			curNdnNode->nextHopNode = oneHopNbr;
			return;
		}
		for (twoHopListIter = twoHopList.begin(); twoHopListIter != twoHopList.end(); twoHopListIter++) {
			twoHopNbrName = &(*twoHopListIter)->prefixName;
			twoHopNbrPreStr = (*twoHopListIter)->prefixName.toUri();
			twoHopNbrStr = (*twoHopListIter)->nodeName;
			// If two hop nbr is the source then continue
			if(prefixStr == twoHopNbrPreStr) {
				//cout << "\t\tThe two hop Nbr " << twoHopNbrStr << " and the source are the same \n";
				continue;
			}
			cout << "\t\t" << "2HopNbr " << twoHopNbrStr << " : " << twoHopNbrPreStr << "\n";
			tmpTrie.insert((*twoHopNbrName), Create < ndn::detail::RegisteredPrefixEntry > (twoHopNbrName));
			curNdnNode->nbrTrie = &tmpTrie;
		}
	}
	if (found != 1) {
		item = (*(curNdnNode->nbrTrie)).longest_prefix_match(*(destPrefixName));
		// If no prefix match with destination then parent node is the nbr
		if (item == 0) {
			//cout << "The original prefix" << srcPrefixName <<"\t" << i;
			//print_name(*srcPrefixName);
			item = (*(curNdnNode->nbrTrie)).longest_prefix_match(*srcPrefixName);
		}
		// if item is still 0 then ideally assert
		//if (item != 0) {
			foundPrefStr = *((item->payload ())->GetPrefix());
			cout << "Longest Prefix found for destination " << destPrefix << " is " << foundPrefStr << endl;
			oneHopList1 = curNdnNode->oneHopList;
			for(oneHopInfoListIter1 = oneHopList1.begin() ; oneHopInfoListIter1 != oneHopList1.end() ; oneHopInfoListIter1++ ) {
				twoHopList1 = (*oneHopInfoListIter1)->oneHopList;
				for (twoHopListIter1 = twoHopList1.begin(); twoHopListIter1 != twoHopList1.end(); twoHopListIter1++) {
					if ((*twoHopListIter1)->prefixName.compare(foundPrefStr) == 0) {
							cout << "Next hop is " << (*oneHopInfoListIter1)->nodeName << endl;
							curNdnNode->nextHopNode = (*oneHopInfoListIter1)->pNode;
							found = 1;
							break;
					}
				}
				if (found == 1)
					break;
			}
		//}
	}
	cout << "\n-------------------------------------------------\n\n\n";
}

void AddPath(unsigned firstNode,unsigned SecndNode, int metric, string str){
	Ptr<Node> node1=NodeList::GetNode(firstNode);
	Ptr<Fib>  fib  = node1->GetObject<Fib> ();
	unsigned m, k;
	for(m=0; m<node1->GetNDevices(); m++){
		Ptr<Channel> ch=node1->GetDevice(m)->GetChannel();
		for(k=0; k<ch->GetNDevices(); k++){
			if(ch->GetDevice(k)->GetNode()->GetId()==SecndNode){
				//Ptr<Ipv4> stack = node->GetObject<Ipv4> ();
				Ptr<ndn::L3Protocol> l3 = node1->GetObject<ndn::L3Protocol> ();
		       // str="prefix1";
				Ptr<Name> name = Create<Name>(str);
				//Sname->append(str);
				const Ptr<const Name> &prefix=name;
				Ptr<fib::Entry> entry = fib->Add (prefix, l3->GetFace(m), metric );
			}
		}
	}
}


void AddFibEntries (Ptr<Node> curNode) {
	Ptr<Node> srcNode;
	Ptr<Node> nextHopNode;
	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);

	Ptr<ndn::GlobalRouter> source;

	srcNode = curNode;
	nextHopNode = curNdnNode->nextHopNode;
	AddPath(srcNode->GetId(), nextHopNode->GetId(), 1, interestPrefixstr);
}

void DeleteTree(Ptr<Node> curNode)
{
	Ptr<NdnNode> curNdnNode = GetNdnNodefromId(curNode->GetId()) ;
	//cout << curNdnNode->nodeName << "\n";
	//if (curNdnNode->nbrTrie)
		//delete curNdnNode->nbrTrie;
	//cout << "as32423 \n";

}

unsigned int HashFunction(unsigned int curNodeId)
{
	return curNodeId % anchorsCnt;
}

void IdentifyAnchors()
{
	unsigned i;
	cout << "anchorsCnt " << anchorsCnt << "\n";
	for (i = 0; i < anchorsCnt; i++)
		cout  << "i" << i << "\n" ;
		anchorList.push_back(GetNdnNodefromId(i));
}


int main (int argc, char *argv[])
{
	routetype_t routeMethod = AIR;
	string statsfile = "scratch/outfile_air";

	// Setting default parameters for PointToPoint links and channels
	Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1Mbps"));
	Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
    Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("10"));

	// Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
	CommandLine cmd;
	cmd.Parse (argc, argv);
	if (argv[1] && !strcmp(argv[1], "dij")) {
		routeMethod = DIJKSTRA;
		statsfile = "scratch/outfile_dij";
	}
	if (argv[1] && !strcmp(argv[1], "air")) {
		routeMethod = AIR;
		statsfile = "scratch/outfile_air";
	}
	if (argv[2]) {
		prod = atoi(argv[2]);
		statsfile = statsfile + argv[2];
		cout << "prod " << prod << "\n";
		cout << "=================================" << "\n";
	}

	// Read the topology from the topology text file
	topologyReader.SetFileName("scratch/paper_topo.txt");
	topologyReader.Read();

	// Get the nodes in nodeContainer
	NodeContainer nodeContainer = NodeContainer::GetGlobal();



	//fill_names();
	//fill_nbr_table();
	//add_node_identifiers();


	// Install NDN stack on all nodes
	ndn::StackHelper ndnHelper;
	ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
	ndnHelper.InstallAll ();

	// Installing global routing interface on all nodes

	ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
	ndnGlobalRoutingHelper.InstallAll ();

	// Getting containers for the consumer/producer
	NodeContainer producerNodes;
	producerNodes.Add (nodeContainer.Get (prod));

	//print_nbr_table();
	NodeContainer consumerNodes;
	for (unsigned i = 0; i < NODE_CNT; i++)
		consumerNodes.Add (nodeContainer.Get (i));

	ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
	consumerHelper.SetPrefix (interestPrefixstr);
	consumerHelper.SetAttribute ("Frequency", StringValue ("1")); // 10 interests a second
	consumerHelper.Install (consumerNodes);

	ndn::AppHelper producerHelper ("ns3::ndn::Producer");
	producerHelper.SetPrefix (interestPrefixstr);
	producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
	producerHelper.Install (producerNodes);

	// Add /prefix origins to ndn::GlobalRouter
	ndnGlobalRoutingHelper.AddOrigins (interestPrefixstr, producerNodes);

	// Normal Dijkstra's routing. Uncomment for testing
	if (routeMethod == DIJKSTRA) {
		ndn::GlobalRoutingHelper::CalculateRoutes ();
	}
	// Start AIR Routing
	else {
		CreateNodeContainer();
		AllNodesCall(FillOneHopNbrList, NDN_INCREASING_NODE_ID);
		GetRootId();
		AllNodesCall(FillChildrenList, NDN_ROOT_TO_CHILDREN);
		//AllNodesCall(PrintChildren, NDN_ROOT_TO_CHILDREN);
		AllNodesCall(AssignPrefixName, NDN_ROOT_TO_CHILDREN);
		AllNodesCall(FillTwoHopTrie, NDN_INCREASING_NODE_ID);
		IdentifyAnchors();
		AllNodesCall(PublishToAnchor, NDN_ROOT_TO_CHILDREN);
		AllNodesCall(FindNextHop, NDN_INCREASING_NODE_ID);
		AllNodesCall(AddFibEntries, NDN_INCREASING_NODE_ID);
	}
	Simulator::Stop (Seconds (1.0));
	ndn::AppDelayTracer::InstallAll(statsfile);

	Simulator::Run ();
	//AllNodesCall(DeleteTree, NDN_ROOT_TO_CHILDREN);

	cout << "=================================" << "\n";

	Simulator::Destroy ();
	return 0;
}



/*

void print_nbr_table() {
	std::list<NodeInfo * > oneHopNodeInfoList;
	std::list<NodeInfo *>::const_iterator oneHopInfoListIter;
	std::list<Ptr<Node> > oneHopList;
	std::list<Ptr<Node> >::const_iterator oneHopListIter;
	std::list<NodeInfo *> twoHopList;
	std::list<NodeInfo *>::const_iterator twoHopListIter;
	Ptr<Node> oneHopNbr;
	Ptr<Node> twoHopNbr;
	std::string prefixStr;
	std::string sourceName;
	std::string oneHopNbrName;
	std::string twoHopNbrName;
	int i;


	for(i = 0; i != NODE_CNT; i++ ) {
		sourceName = nbrTable[i].nodeName;
		prefixStr = nbrTable[i].prefixStr;
		oneHopNodeInfoList = nbrTable[i].oneHopNodeInfoList;
		for(oneHopInfoListIter = oneHopNodeInfoList.begin() ; oneHopInfoListIter != oneHopNodeInfoList.end() ; oneHopInfoListIter++ ) {
			oneHopNbr = (*oneHopInfoListIter)->node;
			oneHopNbrName = (*oneHopInfoListIter)->nodeName;
			twoHopList = (*oneHopInfoListIter)->oneHopNodeInfoList;
			for (twoHopListIter = twoHopList.begin(); twoHopListIter != twoHopList.end(); twoHopListIter++) {
				twoHopNbrName = (*twoHopListIter)->nodeName;
				//std::cout << "Pri " << sourceName << " " << prefixStr << " -> " <<oneHopNbrName << " -> " << twoHopNbrName <<"\n";
			}
		}
	}
}

void print_name(ndn::Name &namePrefix) {
	ndn::Name::const_iterator i;
	for (i = namePrefix.begin(); i != namePrefix.end(); i++) {
		cout << "/" << *i;
	}
	cout<< "\n";
}


void fill_next_hops() {
	std::list<NodeInfo * > oneHopNodeInfoList;
	std::list<NodeInfo * > oneHopNodeInfoList1;
	std::list<NodeInfo *>::const_iterator oneHopInfoListIter;
	std::list<NodeInfo *>::const_iterator oneHopInfoListIter1;
	std::list<Ptr<Node> > oneHopList;
	std::list<NodeInfo *>::reverse_iterator reviter;
	std::list<Ptr<Node> >::const_iterator oneHopListIter;
	std::list<NodeInfo *> twoHopList;
	std::list<NodeInfo *> twoHopList1;
	std::list<NodeInfo *>::const_iterator twoHopListIter;
	std::list<NodeInfo *>::const_iterator twoHopListIter1;
	Ptr<Node> oneHopNbr;
	Ptr<Node> twoHopNbr;
	std::string prefixStr;
	std::string sourceName;
	std::string oneHopNbrName;
	std::string oneHopNbrPrefix;
	std::string twoHopNbrStr;
	std::string twoHopNbrPreStr;
	ndn::Name foundPrefStr;
	Ptr<ndn::Name> twoHopNbrName;
	std::vector<std::string>::const_iterator namesIter;
	int i = 0;
	std::vector<Ptr<ndn::Name> > tmpPrefix;
	super::iterator item;
	std::string destPrefix = nbrTable[DEST].prefixStr;
	Ptr<ndn::Name> destPrefixName = nbrTable[DEST].prefixName;
	Ptr<Node> nextHop;
	int found = 0;
	Ptr<ndn::Name> srcPrefixName;
	cout<< "Destination Prefix is" << destPrefix << endl;
	cout << "\n-------------------------------------------------\n";

	for(i = 0; i != NODE_CNT; i++ ) {
		found = 0;
		sourceName = nbrTable[i].nodeName;
		srcPrefixName = nbrTable[i].prefixName;
		prefixStr = nbrTable[i].prefixStr;
		cout << "Sourcename " << sourceName << "\n";

		oneHopNodeInfoList = nbrTable[i].oneHopNodeInfoList;
		// If source is the dest then break
		if(prefixStr == destPrefix) {
			cout << "Current node " << sourceName << " is the dest \n";
			found = 1;
			nextHop = 0;
			nbrTable[i].nextHopNode = nbrTable[i].node;
			continue;
		}

		ndn::ndnSIM::trie_with_policy< ndn::Name,
											ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
											ndn::ndnSIM::counting_policy_traits > tmpTrie;

		for(oneHopInfoListIter = oneHopNodeInfoList.begin() ; oneHopInfoListIter != oneHopNodeInfoList.end() ; oneHopInfoListIter++ ) {
			oneHopNbr = (*oneHopInfoListIter)->node;
			oneHopNbrName = (*oneHopInfoListIter)->nodeName;
			oneHopNbrPrefix = (*oneHopInfoListIter)->prefixStr;
			cout << "\t" <<"1HopNbr " << oneHopNbrName << "\n";
			twoHopList = (*oneHopInfoListIter)->oneHopNodeInfoList;
			// If one hop nbr is the dest then break
			if(oneHopNbrPrefix == destPrefix) {
				cout << "Next hop is " << oneHopNbrName << " (which is also the dest) \n";
				found = 1;
				nextHop = oneHopNbr;
				nbrTable[i].nextHopNode = oneHopNbr;
				break;
			}
			for (twoHopListIter = twoHopList.begin(); twoHopListIter != twoHopList.end(); twoHopListIter++) {
				twoHopNbrName = (*twoHopListIter)->prefixName;
				twoHopNbrPreStr = (*twoHopListIter)->prefixStr;
				twoHopNbrStr = (*twoHopListIter)->nodeName;
				// If two hop nbr is the source then continue
				if(prefixStr == twoHopNbrPreStr) {
					//cout << "\t\tThe two hop Nbr " << twoHopNbrStr << " and the source are the same \n";
					continue;
				}
				cout << "\t\t" << "2HopNbr " << twoHopNbrStr << " : " << twoHopNbrPreStr << "\n";
				tmpTrie.insert((*twoHopNbrName), Create < ndn::detail::RegisteredPrefixEntry > (twoHopNbrName));
				nbrTable[i].nbrTrie = &tmpTrie;
			}
		}
		if (found != 1) {
			item = (tmpTrie).longest_prefix_match(*(destPrefixName));
			// If no prefix match with destination then parent node is the nbr
			if (item == 0) {
				//cout << "The original prefix" << srcPrefixName <<"\t" << i;
				//print_name(*srcPrefixName);
				item = (tmpTrie).longest_prefix_match(*srcPrefixName);
			}
			// if item is still 0 then ideally assert
			//if (item != 0) {
				foundPrefStr = *((item->payload ())->GetPrefix());
				cout << "Longest Prefix found is" << foundPrefStr << endl;
				oneHopNodeInfoList1 = nbrTable[i].oneHopNodeInfoList;
				for(oneHopInfoListIter1 = oneHopNodeInfoList1.begin() ; oneHopInfoListIter1 != oneHopNodeInfoList1.end() ; oneHopInfoListIter1++ ) {
					twoHopList1 = (*oneHopInfoListIter1)->oneHopNodeInfoList;
					for (twoHopListIter1 = twoHopList1.begin(); twoHopListIter1 != twoHopList1.end(); twoHopListIter1++) {
						if ((*twoHopListIter1)->prefixName->compare(foundPrefStr) == 0) {
								cout << "Next hop is " << (*oneHopInfoListIter1)->nodeName << endl;
								nbrTable[i].nextHopNode = (*oneHopInfoListIter1)->node;
								found = 1;
								break;
						}
					}
					if (found == 1)
						break;
				}
			//}
		}
		cout << "\n-------------------------------------------------\n";
	}
	cout << "\n-------------------------------------------------\n\n\n";
}

void add_fib_entries (void) {
	int i;
	Ptr<ndn::L3Protocol> ndnProt;
	Ptr<ndn::Face> nextFace = 0;
	std::list<TopologyReader::Link> links;
	links = topologyReader.GetLinks();
	std::list<TopologyReader::Link>::iterator linkiter;
	Ptr<Node> srcNode;
	Ptr<Node> nextHopNode;


	std::string fromName;
	std::string toName;
	std::string nextHopName;
	std::string srcName;
	Ptr<ndn::GlobalRouter> source;

	for (i = 0; i < NODE_CNT; i++) {

		srcNode = nbrTable[i].node;
		nextHopNode = nbrTable[i].nextHopNode;
		add_path(srcNode->GetId(), nextHopNode->GetId(), 1, interestPrefixstr);
	}
}




void print_identifiers (void) {
	int i;
	int size = ndnNodeContainer.size();

	for (i = 0; i < size; i++) {
		//cout << "Node : " << nbrTable[i].nodeId << "\t" << "identifier : " << nbrTable[i].nodeName << endl;
		cout << "NdnNode : " << ndnNodeContainer[i].ndnNodeId << "\t" << "Identifier : " << ndnNodeContainer[i].nodeName << endl;
	}
	cout << "\n\n\n";
}

void fill_names() {
	std::vector<std::string>::const_iterator namesIter;
	int i = 0;

	for (i = 0; i < NODE_CNT; i++) {
		//prefixNames[i](prefixNamesArr[i]);
		prefix.push_back(Create<ndn::Name> (prefixNamesArr[i])); // another way to create name
		names.push_back(prefixNamesArr[i]);
		prefixTrie.insert(*(prefix[i]), Create < ndn::detail::RegisteredPrefixEntry > (prefix[i]));
	}
	// Test
	super::iterator item = prefixTrie.find_exact(*(prefix[4]));
	//cout << "Prefix found" << *((item->payload ())->GetPrefix()) << endl;
	ndn::Name n1("/0/2/1/1/3");
	item = prefixTrie.longest_prefix_match(n1);
	//cout << "Longest Prefix found" << *((item->payload ())->GetPrefix()) << endl;
}

void add_node_identifiers (void) {
	int i;

	for (i = 0; i < NODE_CNT; i++) {
		nbrTable[i].nodeId = ndnNodeIdTable[i];
	}
}

void fill_nbr_table() {
	std::vector<Ptr<Node> >::iterator nodeIter;
	std::vector<std::string>::const_iterator namesIter;
	NodeContainer nodeContainer = NodeContainer::GetGlobal();
	std::list<TopologyReader::Link> links;
	links = topologyReader.GetLinks();
	std::list<TopologyReader::Link>::iterator linkiter;
	std::string fromName = "";
	std::string toName = "";
	int pos;

	for(linkiter = links.begin() ; linkiter != links.end() ; linkiter++ ) {
		from = (*linkiter).GetFromNode();

		fromName = (*linkiter).GetFromNodeName();
		to = (*linkiter).GetToNode();
		toName = (*linkiter).GetToNodeName();
		//std::cout << "Pri : " << fromName << " -> " << toName << " : " << to->GetId() << "\n";
		pos = from->GetId();
		nbrTable[pos].node = from;
		nbrTable[pos].nodeName = fromName;
		nbrTable[pos].prefixStr = names[pos];
		nbrTable[pos].prefixName = prefix[pos];
		nbrTable[pos].oneHopList.push_back(to);
		//std::cout << "Pri : " << toName << " -> " << fromName << " : " << from->GetId() << "\n";
		pos = to->GetId();
		nbrTable[pos].node = to;
		nbrTable[pos].nodeName = toName;
		nbrTable[pos].prefixStr = names[pos];
		nbrTable[pos].prefixName = prefix[pos];
		nbrTable[pos].oneHopList.push_back(from);
	}
	//std::cout << std::endl;
	fill_two_hop_nbr_info();
}


void fill_two_hop_nbr_info() {
	std::list<NodeInfo * > oneHopNodeInfoList;
	std::list<NodeInfo *>::const_iterator oneHopInfoListIter;
	std::list<Ptr<Node> > oneHopList;
	std::list<Ptr<Node> >::const_iterator oneHopListIter;
	Ptr<Node> sourceNode;

	int j;
	int i;

	for(i = 0; i != NODE_CNT; i++ ) {
		  oneHopList = nbrTable[i].oneHopList;
		  for(oneHopListIter = oneHopList.begin() ; oneHopListIter != oneHopList.end() ; oneHopListIter++ ) {
			  for (j = 0; j != NODE_CNT; j++) {
				  if (nbrTable[j].node == *oneHopListIter) {
					  nbrTable[i].oneHopNodeInfoList.push_back(&nbrTable[j]);
				  }
			  }
		  }
	}
}

void add_path(unsigned firstNode,unsigned SecndNode, int metric, string str){
	Ptr<Node> node1=NodeList::GetNode(firstNode);
	Ptr<Fib>  fib  = node1->GetObject<Fib> ();
	unsigned m, k;
	for(m=0; m<node1->GetNDevices(); m++){
		Ptr<Channel> ch=node1->GetDevice(m)->GetChannel();
		for(k=0; k<ch->GetNDevices(); k++){
			if(ch->GetDevice(k)->GetNode()->GetId()==SecndNode){
				//Ptr<Ipv4> stack = node->GetObject<Ipv4> ();
				Ptr<ndn::L3Protocol> l3 = node1->GetObject<ndn::L3Protocol> ();
		       // str="prefix1";
				Ptr<Name> name = Create<Name>(str);
				//Sname->append(str);
				const Ptr<const Name> &prefix=name;
				Ptr<fib::Entry> entry = fib->Add (prefix, l3->GetFace(m), metric );
			}
		}
	}
}


void InstallHelloApp()
{
	NodeContainer nodeContainer = NodeContainer::GetGlobal();
	ndn::AppHelper TxHelper ("ns3::ndn::HelloApp");
	ApplicationContainer txApp = TxHelper.Install (nodeContainer);

	//txApp.Start (Seconds (1.0));
	ndn::AppHelper rxHelper ("ns3::ndn::HelloApp");
	//txApp.Install (nodeContainer.Get (prod));
	//rxHelper.Install (nodeContainer.Get (prod));
	rxHelper.Install (nodeContainer);
}

void OnInterest (Ptr<Face> inFace, Ptr<Interest> interest)
{
	return;
}

void OnData (Ptr<Face> pFace, Ptr<Data> data)
{
	Ptr <const Packet> packet = data->GetPayload();
	//uint8_t buffer[sizeof(NdnPacket)];
	NdnPacket ndnPacket;

	packet->CopyData((uint8_t *)&ndnPacket, sizeof(NdnPacket));

	if (ndnPacket.packetType == GET_PARENT) {
		FindParentThruMsg(pFace, ndnPacket);
	}

	if (ndnPacket.packetType == GET_PREFIXNAME) {
		AssignPrefixNameThruMsg(pFace, ndnPacket);
	}

	return;
}

void RequestPrefixName(Ptr<Node> curNode)
{
	NdnPacket ndnPacket;
	std::list<Ptr<Node> >::iterator oneHopListIter;
	//unsigned int size = oneHopList.size();
  	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
  	Ptr<NdnNode> parentNdnNode = GetNdnNodefromId(curNdnNode->parentId);
	Ptr<Face> pFace;
	Ptr<Data> pData = Create<Data> ();
	Ptr<Packet> pPayload;

	std::list<Ptr<Node> > oneHopList = curNdnNode->oneHopList;

	ndnPacket.packetType = GET_PREFIXNAME;
	ndnPacket.senderId = curNdnNode->ndnNodeId;
	ndnPacket.parentId = curNdnNode->parentId;
	ndnPacket.receiverId = curNdnNode->parentId;

  	//send packet to parent;\n
	//cout << "Packet sent from " << curNdnNode->nodeName << "(id:" << curNdnNode->ndnNodeId <<  ")";
	//cout << " to " << parentNdnNode->nodeName << "(id:" << parentNdnNode->ndnNodeId <<  ")" << "\n";
	pPayload = Create<Packet> ((uint8_t *)&ndnPacket, sizeof(NdnPacket));
	pData->SetPayload(pPayload);
	pFace = GetFace(curNode->GetId(), curNdnNode->parentId);
	//pFace->UnRegisterProtocolHandlers();
	//pFace->RegisterProtocolHandlers (MakeCallback (&OnInterest), MakeCallback (&OnData));
	//pFace->ReceiveData(pData);
	OnData(pFace, pData);
}


void AssignPrefixNameThruMsg(Ptr<Face> pFace, NdnPacket ndnPacket) {
	Ptr<NdnNode> childNdnNode;
	Ptr<NdnNode> parentNdnNode;
	int childId;
	std::stringstream sstream;
	string strChildId;
	ndn::Name preName;

	childNdnNode = GetNdnNodefromId(ndnPacket.senderId);
	parentNdnNode = GetNdnNodefromId(ndnPacket.receiverId);

	preName = childNdnNode->prefixName;
	childId = GetChildId(parentNdnNode, childNdnNode);
	sstream << childId;
	strChildId = sstream.str();
	// If parent has a name already then assign a name to the child
	if (parentNdnNode->prefixName != initialPrefixName)
	{
		childNdnNode->prefixName = parentNdnNode->prefixName.append(strChildId);
	}
	// If root then assign name to root prefix name
	if (parentNdnNode->parentId == parentNdnNode->ndnNodeId) {
		parentNdnNode->prefixName = rootPrefixName;
		childNdnNode->prefixName = parentNdnNode->prefixName.append(strChildId);
	}

	NotifyNameChange(childNdnNode->ndnNodeId, preName, childNdnNode->prefixName);
	return;
}



void SendHello(Ptr<Node> curNode)
{
	NdnPacket ndnPacket;
	std::list<Ptr<Node> >::iterator oneHopListIter;
	//unsigned int size = oneHopList.size();
  	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
  	Ptr<NdnNode> nbrNdnNode;
	Ptr<Node> nbrNode;
	Ptr<Face> pFace;
	Ptr<Data> pData = Create<Data> ();
	Ptr<Packet> pPayload;

	std::list<Ptr<Node> > oneHopList = curNdnNode->oneHopList;

	ndnPacket.packetType = GET_PARENT;
	ndnPacket.senderId = curNdnNode->ndnNodeId;
	ndnPacket.parentId = curNdnNode->parentId;


  	//send packet to all its neighbours;
	for(oneHopListIter = oneHopList.begin() ; oneHopListIter != oneHopList.end() ; oneHopListIter++ ) {

		nbrNode = *(oneHopListIter);
		nbrNdnNode = GetNdnNodefromNode(nbrNode);
		//cout << "Packet sent from " << curNdnNode->nodeName << "(id:" << curNdnNode->ndnNodeId <<  ")";
		//cout << " to " << nbrNdnNode->nodeName << "(id:" << nbrNdnNode->ndnNodeId <<  ")" << "\n";
		ndnPacket.receiverId = nbrNdnNode->ndnNodeId;

		pPayload = Create<Packet> ((uint8_t *)&ndnPacket, sizeof(NdnPacket));
		pData->SetPayload(pPayload);
		pFace = GetFace(curNode->GetId(), nbrNode->GetId());
		pFace->RegisterProtocolHandlers (MakeCallback (&OnInterest), MakeCallback (&OnData));
		//pFace->SendData(&data);
		pFace->ReceiveData(pData);
		//ReceiveHello(pFace, pdata);
	}
}

void FindParentThruMsg(Ptr<Face> pFace, NdnPacket ndnPacket) {
	Ptr<NdnNode> curNdnNode;
	Ptr<NdnNode> fromNdnNode;

	fromNdnNode = GetNdnNodefromId(ndnPacket.senderId);
	curNdnNode = GetNdnNodefromId(ndnPacket.receiverId);

		// if sender parentId is less than current parentId then change parentId
	if (ndnPacket.parentId < curNdnNode->parentId) {
		NotifyParentChange(curNdnNode->ndnNodeId, curNdnNode->parentId, ndnPacket.parentId);
		curNdnNode->parentId = ndnPacket.parentId;
		// Parent updated so invalidate prefixName
		curNdnNode->prefixName = invalidPrefixName;
	}
}


void recieve_packet(Ptr<NdnNode> curNdnNode)
{
	NdnPacket ndnPacket;
	std::list<Ptr<Node> > oneHopList;
	std::list<Ptr<Node> >::iterator oneHopListIter;
	//unsigned int size = oneHopList.size();
	Ptr<Node> nbrNode;
	Ptr<Node> curNode = curNdnNode->pNode;
	Ptr<Face> face;
	Data data;
	Packet payload;


	payload = data.SetPayload(&payload);

	//send packet to all its neighbours;
	for(oneHopListIter = oneHopList.begin() ; oneHopListIter != oneHopList.end() ; oneHopListIter++ ) {
		nbrNode = *oneHopListIter;
		face = get_face(curNode->GetId(), nbrNode->GetId());
		face->SendData(&data);
	}
}


void fill_labels()
{
		std::vector<NdnNode>::iterator ndnNodeIter;
		Ptr<NdnNode> curNdnNode;
		for (unsigned int i = 0; i < ndnNodeContainer.size(); i++)
		{
			curNdnNode = &ndnNodeContainer[i];
			if (curNdnNode->prefixName == NULL)
			{
				//send_packet(curNdnNode);
			}
		}

}



void FindParent(Ptr<Node> curNode)
{
	std::list<Ptr<Node> >::iterator oneHopListIter;
  	Ptr<NdnNode> curNdnNode = GetNdnNodefromNode(curNode);
  	Ptr<NdnNode> nbrNdnNode;
	Ptr<Node> nbrNode;
	int minId = curNdnNode->ndnNodeId;

	std::list<Ptr<Node> > oneHopList = curNdnNode->oneHopList;

	//send packet to all its neighbours;
	for(oneHopListIter = oneHopList.begin() ; oneHopListIter != oneHopList.end() ; oneHopListIter++ ) {

		nbrNode = *(oneHopListIter);
		nbrNdnNode = GetNdnNodefromNode(nbrNode);

		if (minId > nbrNdnNode->ndnNodeId)
		{
			minId = nbrNdnNode->ndnNodeId;
		}
	}
	NotifyParentChange(curNdnNode->ndnNodeId, curNdnNode->parentId,  minId);
	curNdnNode->parentId = minId;
	// check if curNode is root Node
	if (curNdnNode->parentId == curNdnNode->ndnNodeId){
		cout << "<!!Notification!!> I am " << curNdnNode->nodeName << "(id:" << curNdnNode->ndnNodeId <<  ")";
		cout << " and I'm the root! \n";
	}
}

// Iterative method to find height of Bianry Tree
void PrintLevelOrder()
{
  	Ptr<NdnNode> rootNdnNode = GetNdnNodefromId(rootId);
	std::list<Ptr<NdnNode> >::iterator childrenListIter;
  	Ptr<NdnNode> childNdnNode;
	int j = 0;

	// Base Case
    if (rootNdnNode == NULL)  return;

    // Create an empty queue for level order tarversal
    queue <Ptr<NdnNode> > q;

    // Enqueue Root and initialize height
    q.push(rootNdnNode);

    while (q.empty() == false)
    {
		// Print front of queue and remove it from queue
        Ptr<NdnNode> tmpNdnNode = q.front();
        cout << tmpNdnNode->nodeName << " ";
		std::list<Ptr<NdnNode> > childrenList = tmpNdnNode->childrenList;
		for(childrenListIter = childrenList.begin() ; childrenListIter != childrenList.end() ; childrenListIter++ ) {
			q.push(*(childrenListIter));
			//cout << "\t\t\t\t\t" << childNdnNode->nodeName << "(id:" << childNdnNode->ndnNodeId <<  ")\n";
			j++;
		}
		q.pop();
    }
}

*/
