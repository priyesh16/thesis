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

#include "myndn.h"


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
std::vector<NodeInfo> nbrTable(NODE_CNT);


//NodeContainer nodeContainer;

ndn::ndnSIM::trie_with_policy< ndn::Name,
					ndn::ndnSIM::smart_pointer_payload_traits<ndn::detail::RegisteredPrefixEntry>,
					ndn::ndnSIM::counting_policy_traits> prefixTrie;

std::vector<Ptr<ndn::Name> > prefix;



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

void create_node_container() {
	std::vector<std::string>::const_iterator namesIter;
	NodeContainer nodeContainer = NodeContainer::GetGlobal();
	std::list<TopologyReader::Link> links;
	links = topologyReader.GetLinks();
	std::list<TopologyReader::Link>::iterator linkiter;
	std::string fromName = "";
	std::string toName = "";
	int pos;
	int topoId;
	int size = nodeContainer.GetN();
	ndnNodeContainer.resize(size);

	for(linkiter = links.begin() ; linkiter != links.end() ; linkiter++ ) {

		from = (*linkiter).GetFromNode();
		fromName = (*linkiter).GetFromNodeName();
		to = (*linkiter).GetToNode();
		toName = (*linkiter).GetToNodeName();

		//std::cout << "Pri : " << fromName << "\t\t -> " << toName << " : " << to->GetId() << " " << pos << "\n";
		topoId = from->GetId();
		pos = ndnNodeIdTable[topoId];
		ndnNodeContainer[pos].pNode = from;
		ndnNodeContainer[pos].nodeName = fromName;
		ndnNodeContainer[pos].oneHopList.push_back(to);
		ndnNodeContainer[pos].ndnNodeId = ndnNodeIdTable[topoId];
		ndnNodeContainer[pos].parentId = ndnNodeIdTable[topoId];
		//ndnNodeContainer[pos].pHelloApp = NULL;

		//std::cout << "Pri : " << toName << "\t\t -> " << fromName << " : " << from->GetId() << " " << pos << "\n";
		topoId = to->GetId();
		pos = ndnNodeIdTable[topoId];
		ndnNodeContainer[pos].pNode = to;
		ndnNodeContainer[pos].nodeName = toName;
		ndnNodeContainer[pos].oneHopList.push_back(from);
		ndnNodeContainer[pos].ndnNodeId = ndnNodeIdTable[topoId];
		ndnNodeContainer[pos].parentId = ndnNodeIdTable[topoId];
		//ndnNodeContainer[pos].pHelloApp = NULL;
	}

	//std::cout << std::endl;
	fill_two_hop_nbr_info();
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

/*
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
*/

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
	int pos;
	int topoId;
  
	topoId = curNode->GetId();
	pos = ndnNodeIdTable[topoId];
	
	return &ndnNodeContainer[pos];
}

Ptr<NdnNode> GetNdnNodefromId(int ndnNodeId)
{

	int i; 
	int flag = 0;

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

void InstallHelloApp()
{
	NodeContainer nodeContainer = NodeContainer::GetGlobal();
	ndn::AppHelper TxHelper ("ns3::ndn::HelloApp");
	ApplicationContainer txApp = TxHelper.Install (nodeContainer);

	//txApp.Start (Seconds (1.0));
	ndn::AppHelper rxHelper ("ns3::ndn::HelloApp");
	//txApp.Install (nodeContainer.Get (PROD));
	//rxHelper.Install (nodeContainer.Get (PROD));
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
		FindParent(pFace, ndnPacket);
	}

	if (ndnPacket.packetType == GET_PREFIXNAME) {
		AssignPrefixName(pFace, ndnPacket);
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

  	//send packet to parent;
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

int GetChildId(Ptr<NdnNode> parentNdnNode, Ptr<NdnNode> childNdnNode)
{
	std::list<Ptr<Node> >::iterator oneHopListIter;
  	Ptr<NdnNode> nbrNdnNode;
	Ptr<Node> nbrNode;
	Ptr<Face> pFace;
	int childId = 0;
	
	std::list<Ptr<Node> > oneHopList = parentNdnNode->oneHopList;

	for(oneHopListIter = oneHopList.begin() ; oneHopListIter != oneHopList.end() ; oneHopListIter++ ) {
		nbrNode = *(oneHopListIter);
		nbrNdnNode = GetNdnNodefromNode(nbrNode);
		if (childNdnNode->ndnNodeId == nbrNdnNode->ndnNodeId) {
			break;
		}
		childId++;
	}
	return childId;
}

void AssignPrefixName(Ptr<Face> pFace, NdnPacket ndnPacket) {
	Ptr<NdnNode> childNdnNode;
	Ptr<NdnNode> parentNdnNode;
	int childId;
	ndn::Name childName;
	
	childNdnNode = GetNdnNodefromId(ndnPacket.senderId);
	parentNdnNode = GetNdnNodefromId(ndnPacket.receiverId);

	// If parent has a name already then assign a name to the child
	if (parentNdnNode->prefixName != NULL)
	{
		childId = GetChildId(parentNdnNode, childNdnNode);
		childName = parentNdnNode->prefixName->appendNumber(childId);
		cout << childName.toUri();
	}
	// If root then assign name to zero
	if (parentNdnNode->parentId == parentNdnNode->ndnNodeId) {
		parentNdnNode->prefixName->appendNumber(0);
	}


	cout << "<!!Notification!!> I am " << childNdnNode->nodeName << "(id:" << childNdnNode->ndnNodeId <<  ")";
	cout << "and I'm changing my name from " << childNdnNode->prefixName; 
	cout << " to " << " "  << "\n";
	childNdnNode->parentId = ndnPacket.parentId;
}

void FindParent(Ptr<Face> pFace, NdnPacket ndnPacket) {
	Ptr<NdnNode> curNdnNode;
	Ptr<NdnNode> fromNdnNode;
	
	fromNdnNode = GetNdnNodefromId(ndnPacket.senderId);
	curNdnNode = GetNdnNodefromId(ndnPacket.receiverId);

		// if sender parentId is less than current parentId then change parentId
	if (ndnPacket.parentId < curNdnNode->parentId) {
		cout << "<!!Notification!!> I am " << curNdnNode->nodeName << "(id:" << curNdnNode->ndnNodeId <<  ")";
		cout << "and I'm changing my parent from " << curNdnNode->parentId; 
		cout << " to " << ndnPacket.parentId  << "\n";
		curNdnNode->parentId = ndnPacket.parentId;
		// Parent updated so invalidate prefixName
		curNdnNode->prefixName = Create<ndn::Name> ("/invalid"); // another way to create name;
	}
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


int main (int argc, char *argv[])
{
	// Setting default parameters for PointToPoint links and channels
	Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1Mbps"));
	Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
    Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("10"));

	// Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
	CommandLine cmd;
	cmd.Parse (argc, argv);

	// Read the topology from the topology text file
	topologyReader.SetFileName("scratch/paper_topo.txt");
	topologyReader.Read();

	// Get the nodes in nodeContainer
	NodeContainer nodeContainer = NodeContainer::GetGlobal();

	// create the node container
	create_node_container();

	fill_names();
	fill_nbr_table();
	add_node_identifiers();
	

	// Install NDN stack on all nodes
	ndn::StackHelper ndnHelper;
	ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
	ndnHelper.InstallAll ();

	// Installing global routing interface on all nodes

	ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
	ndnGlobalRoutingHelper.InstallAll ();

	// Getting containers for the consumer/producer
	Ptr<Node> producer = nodeContainer.Get (PROD);
	//print_nbr_table();
	NodeContainer consumerNodes;
	consumerNodes.Add (nodeContainer.Get (CONS));

	ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
	consumerHelper.SetPrefix (interestPrefixstr);
	consumerHelper.SetAttribute ("Frequency", StringValue ("1")); // 10 interests a second
	consumerHelper.Install (consumerNodes);

	ndn::AppHelper producerHelper ("ns3::ndn::Producer");
	producerHelper.SetPrefix (interestPrefixstr);
	producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
	producerHelper.Install (producer);

	fill_next_hops();

	// Add /prefix origins to ndn::GlobalRouter
	ndnGlobalRoutingHelper.AddOrigins (interestPrefixstr, producer);

	// Calculate and install FIBs
	add_fib_entries();
	//ndn::GlobalRoutingHelper::CalculateRoutes ();
	//print_identifiers();
	SendHello(nodeContainer.Get (12));
	RequestPrefixName(nodeContainer.Get (13));
	//InstallHelloApp(nodeContainer.Get (PROD));

	Simulator::Stop (Seconds (10.0));
	ndn::AppDelayTracer::InstallAll("outfile.txt");
	Simulator::Run ();
	Simulator::Destroy ();

	 return 0;
}
