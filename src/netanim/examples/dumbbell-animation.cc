/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: George F. Riley<riley@ece.gatech.edu>
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1500));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (2));

  uint32_t    nLeftLeaf = 5;
  uint32_t    nRightLeaf = 5;
  uint32_t    nLeaf = 0; // If non-zero, number of both left and right
  std::string animFile = "dumbbell-animation.xml" ;  // Name of file for animation output
  std::string transport_prot = "TcpWestwoodPlus";
  double bursterror = 0.001;
  std::string pcapFile = "dumbbell-westwoodplus-bursterror-3" ;  // Name of file for animation output
  std::string sack="no";


  CommandLine cmd;
  cmd.AddValue ("transport_prot", "Transport protocol to use: TcpNewReno, "
                " TcpWestwood, TcpWestwoodPlus ", transport_prot);
  cmd.AddValue ("nLeftLeaf", "Number of left side leaf nodes", nLeftLeaf);
  cmd.AddValue ("nRightLeaf","Number of right side leaf nodes", nRightLeaf);
  cmd.AddValue ("nLeaf",     "Number of left and right side leaf nodes", nLeaf);
  cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);
  cmd.AddValue ("pcapFile",  "File Name for Pcap Output", pcapFile);
  cmd.AddValue ("error",  "Burst error rate", bursterror);
  cmd.AddValue ("sack",  "Sack enable or not", sack);

  SeedManager::SetSeed (1);
  SeedManager::SetRun (10);

  cmd.Parse (argc,argv);
  if (nLeaf > 0)
    {
      nLeftLeaf = nLeaf;
      nRightLeaf = nLeaf;
    }


  // Select TCP variant
  if (transport_prot.compare ("TcpNewReno") == 0)
    {
      std::cout << "Tcp NewReno" << std::endl;
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
    }
  else if (transport_prot.compare ("TcpWestwoodPlus") == 0)
    {
      std::cout << "Tcp WestwoodPlus" << std::endl;
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
      Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
      Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
    }
  else if (transport_prot.compare ("TcpWestwood") == 0)
    {
      std::cout << "Tcp Westwood" << std::endl;
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
      Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
    }
  else
    {
      exit (1);
    }

  if (sack.compare("yes") == 0)
    {
      std::cout << "SACK Enable" << std::endl;
      Config::SetDefault ("ns3::TcpSocketBase::SACK", BooleanValue (true));   
    }

  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  Ptr<UniformRandomVariable> vu = CreateObject<UniformRandomVariable> ();
  vu->SetStream (0);
  BurstErrorModel error_model;
  error_model.SetBurstRate(bursterror);
  error_model.SetRandomVariable(uv);
  uv->SetStream (2);
  error_model.SetRandomBurstSize(uv);

  // Create the point-to-point link helpers
  PointToPointHelper pointToPointRouter;
  pointToPointRouter.SetDeviceAttribute  ("DataRate", StringValue ("4Mbps"));
  pointToPointRouter.SetChannelAttribute ("Delay", StringValue ("50ms"));
  pointToPointRouter.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (&error_model));

  PointToPointHelper pointToPointLeaf;
  pointToPointLeaf.SetDeviceAttribute    ("DataRate", StringValue ("10Mbps"));
  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("0.1ms"));



  PointToPointDumbbellHelper d (nLeftLeaf, pointToPointLeaf,
                                nRightLeaf, pointToPointLeaf,
                                pointToPointRouter);

  // Install Stack
  InternetStackHelper stack;
  d.InstallStack (stack);

  // Assign IP Addresses
  d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

  // Install on/off app on all right side nodes
  BulkSendHelper clientHelper ("ns3::TcpSocketFactory", Address ());
  ApplicationContainer clientApps;

  for (uint32_t i = 0; i < d.LeftCount (); ++i)
    {
      // Create an on/off app sending packets to the same leaf right side
      AddressValue remoteAddress (InetSocketAddress (d.GetRightIpv4Address (i), 1000));
      clientHelper.SetAttribute ("Remote", remoteAddress);
      clientApps.Add (clientHelper.Install (d.GetLeft (i)));
    }

  clientApps.Start (Seconds (0.0));
  clientApps.Stop (Seconds (600.0));

  //sink for servers
    PacketSinkHelper sink ("ns3::TcpSocketFactory",Address
                           (InetSocketAddress (Ipv4Address::GetAny (), 1000)));
    //set nodes as server
     ApplicationContainer serverApps;
    for (uint32_t i = 0; i < d.RightCount (); ++i)
     {
       serverApps.Add(sink.Install (d.GetRight(i)));
     }

    serverApps.Start (Seconds (0.0));
    serverApps.Stop (Seconds (600.0));
  // Set the bounding box for animation
  d.BoundingBox (1, 1, 100, 100);
  //pointToPointRouter.EnablePcapAll ("dumbell-router");
  pointToPointLeaf.EnablePcapAll ("dumbbell/"+pcapFile);
  // Create the animation object and configure for specified output
  /*AnimationInterface anim (animFile);
  anim.SetMaxPktsPerTraceFile(500000);
  anim.EnablePacketMetadata (); // Optional++
  anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (600)); // Optional*/
  
  // Set up the acutal simulation
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Simulator::Run ();
  std::cout << "Animation Trace file created:" << animFile.c_str () << std::endl;
  Simulator::Destroy ();
  return 0;
}
