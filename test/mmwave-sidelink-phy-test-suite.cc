/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2020 University of Padova, Dep. of Information Engineering,
*   SIGNET lab.
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ns3/mmwave-sidelink-spectrum-phy.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/isotropic-antenna-model.h"
#include "ns3/spectrum-helper.h"
#include "ns3/mmwave-spectrum-value-helper.h"
#include "ns3/test.h"
#include "ns3/mmwave-sidelink-phy.h"
#include "ns3/mmwave-vehicular-net-device.h"

NS_LOG_COMPONENT_DEFINE ("MmWaveVehicularSpectrumPhyTestSuite");

using namespace ns3;
using namespace millicar;

/**
 * In this test, two packets are sent from the tx node the rx node.
 */
class MmWaveVehicularSpectrumPhyTestCase1 : public TestCase
{
public:
  /**
   * Constructor
   */
  MmWaveVehicularSpectrumPhyTestCase1 ();

  /**
   * Destructor
   */
  virtual ~MmWaveVehicularSpectrumPhyTestCase1 ();

private:
  /**
   * Struct used to store the test parameters
   */
  struct TestVector
  {
    double distance; //!< distance between tx and rx
    Time ipi; //!< interpacket interval
  };

  /**
   * This method creates the test vectors and calls StartTest
   */
  virtual void DoRun (void);

  /**
   * This method runs the simulation using the specified parameters
   * \param testVector the TestVector instance containing the parameters
   */
  void StartTest (TestVector testVector);

  /**
   * Periodically transmit a dummy packet burst containing a single packet
   * \param tx_phy the tx PHY instance
   * \param ipi the interpacket interval
   */
  void Tx (Ptr<MmWaveSidelinkPhy> tx_phy, Time ipi);

  /**
   * This method is a callback sink which is fired when the rx receives a packet
   * \param p received packet
   */
  void Rx (Ptr<Packet> p);

  /**
   * This method is a callback sink which is fired when the rx updates the SINR
   * estimate
   * \param sinr the sinr value
   */
  void UpdateSinrPerceived (const SpectrumValue& sinr);

  uint32_t m_tx; //!< tx packet counter
  uint32_t m_rx; //!< rx packet counter
  std::vector<double> m_sinr; //!< stores the perceived sinrs for each packet
};

MmWaveVehicularSpectrumPhyTestCase1::MmWaveVehicularSpectrumPhyTestCase1 ()
  : TestCase ("MmwaveVehicular test case (does nothing)")
{
}

MmWaveVehicularSpectrumPhyTestCase1::~MmWaveVehicularSpectrumPhyTestCase1 ()
{
}

void
MmWaveVehicularSpectrumPhyTestCase1::Tx (Ptr<MmWaveSidelinkPhy> tx_phy, Time ipi)
{
  // create a dummy packet burst
  Ptr<Packet> p = Create<Packet> (1024); //TODO how to set the size?
  Ptr<PacketBurst> pb1 = CreateObject<PacketBurst> ();
  pb1->AddPacket (p);

  // create the associated DCI
  mmwave::DciInfoElementTdma dci;
  dci.m_mcs = 0; // dummy value
  dci.m_tbSize = p->GetSize (); // dummy value
  dci.m_symStart = 0; // dummy value
  dci.m_numSym = 3; // dummy value
  dci.m_rnti = 1; // RNTI of the TX node

  // create the mmwave::SlotAllocInfo containing the transmission information
  mmwave::SlotAllocInfo info;
  info.m_slotType = mmwave::SlotAllocInfo::DATA;
  info.m_slotIdx = 0;
  info.m_dci = dci;
  info.m_rnti = 2; // the RNTI of the destination node

  tx_phy->DoAddTransportBlock (pb1, info);

  NS_LOG_DEBUG ("Tx packet of size " << p->GetSize ());

  Simulator::Schedule (ipi, &MmWaveVehicularSpectrumPhyTestCase1::Tx, this, tx_phy, ipi);

  ++m_tx;
}

void
MmWaveVehicularSpectrumPhyTestCase1::Rx (Ptr<Packet> p)
{
  NS_LOG_DEBUG ("Rx packet of size " << p->GetSize ());
  ++m_rx;
}

void
MmWaveVehicularSpectrumPhyTestCase1::UpdateSinrPerceived (const SpectrumValue& sinr)
{
  double actualSnr = 10 * log10 (Sum (sinr) / sinr.GetSpectrumModel ()->GetNumBands ());
  m_sinr.push_back (actualSnr);
  NS_LOG_DEBUG ("SINR " << actualSnr << " dB" );
}

void
MmWaveVehicularSpectrumPhyTestCase1::DoRun (void)
{
  std::list<TestVector> tests;

  TestVector test1;
  test1.distance = 400;
  test1.ipi = MicroSeconds (1000); // subframe period is 800 us in default configuration
  tests.push_back (test1);

  TestVector test2;
  test2.distance = 450;
  test2.ipi = MicroSeconds (1000);
  tests.push_back (test2);

  TestVector test3;
  test3.distance = 500;
  test3.ipi = MicroSeconds (1000);
  tests.push_back (test3);

  TestVector test4;
  test4.distance = 550;
  test4.ipi = MicroSeconds (1000);
  tests.push_back (test4);

  TestVector test5;
  test5.distance = 600;
  test5.ipi = MicroSeconds (1000);
  tests.push_back (test5);

  for (auto t : tests)
  {
    // reset the counters
    m_rx = 0;
    m_tx = 0;
    m_sinr.clear ();

    // perform the test
    StartTest (t);
  }
}

void
MmWaveVehicularSpectrumPhyTestCase1::StartTest (TestVector testVector)
{
  double distance = testVector.distance; // distance between tx and rx
  Time ipi =testVector.ipi; // interpacket interval

  // create the tx and rx nodes
  NodeContainer n;
  n.Create (2); // node 0 is tx, node 1 is rx

  // create the tx mobility model and aggregate it with the tx node
  Ptr<MobilityModel> tx_mm = CreateObject<ConstantPositionMobilityModel> ();
  tx_mm->SetPosition (Vector (0.0, 0.0, 0.0));
  n.Get (0)->AggregateObject (tx_mm);

  // create the rx mobility model and aggregate it with the rx node
  Ptr<MobilityModel> rx_mm = CreateObject<ConstantPositionMobilityModel> ();
  rx_mm->SetPosition (Vector (distance, 0.0, 0.0));
  n.Get (1)->AggregateObject (rx_mm);

  // create the tx and rx antenna
  Ptr<AntennaModel> tx_am = CreateObject<IsotropicAntennaModel> ();
  Ptr<AntennaModel> rx_am = CreateObject<IsotropicAntennaModel> ();

  // create the channel
  SpectrumChannelHelper sh = SpectrumChannelHelper::Default ();
  Ptr<SpectrumChannel> sc = sh.Create ();

  // create the configuration
  Ptr<mmwave::MmWavePhyMacCommon> pmc = CreateObject<mmwave::MmWavePhyMacCommon> ();
  double subcarrierSpacing = 15 * std::pow (2, 2) * 1000; // subcarrier spacing based on the numerology. Only 60KHz and 120KHz is supported in NR V2X.
  pmc->SetSymbPerSlot(14); // TR 38.802 Section 5.3: each slot must have 14 symbols < Symbol duration is dependant on the numerology
  pmc->SetSlotPerSubframe(std::pow (2, 2)); // flexible number of slots per subframe - depends on numerology
  pmc->SetSubframePeriod (1000); // TR 38.802 Section 5.3: the subframe duration is 1ms, i.e., 1000us, and the frame length is 10ms.
  double symbolPeriod = pmc->GetSubframePeriod () / pmc->GetSlotsPerSubframe () / 14.0;
  pmc->SetSymbolPeriod (symbolPeriod); // symbol period is required in microseconds
  double subCarriersPerRB = 12;
  pmc->SetNumChunkPerRB(1); // each resource block contains 1 chunk
  pmc->SetNumRb ( uint32_t( 100e6 / (subcarrierSpacing * subCarriersPerRB) ) );
  pmc->SetChunkWidth (subCarriersPerRB*subcarrierSpacing);

  // create and configure the tx spectrum phy
  Ptr<MmWaveSidelinkSpectrumPhy> tx_ssp = CreateObject<MmWaveSidelinkSpectrumPhy> ();
  tx_ssp->SetMobility (tx_mm);
  tx_ssp->SetAntenna (tx_am);
  tx_ssp->SetChannel (sc);

  // create the tx phy
  Ptr<MmWaveSidelinkPhy> tx_phy = CreateObject<MmWaveSidelinkPhy> (tx_ssp, pmc);

  // create and configure the tx spectrum phy
  Ptr<MmWaveSidelinkSpectrumPhy> rx_ssp = CreateObject<MmWaveSidelinkSpectrumPhy> ();
  rx_ssp->SetMobility (rx_mm);
  rx_ssp->SetAntenna (rx_am);
  rx_ssp->SetChannel (sc);

  // creating mac instance in order to be able to call the method DoSlotIndication from MmWaveSidelinkPhy::StartSlot()
  Ptr<MmWaveSidelinkMac> tx_mac = CreateObject<MmWaveSidelinkMac> (pmc);
  tx_mac->SetRnti(1);
  tx_phy->SetPhySapUser(tx_mac->GetPhySapUser());

  // add the rx spectrum phy instance to the spectrum channel
  sc->AddRx (rx_ssp);

  // create the rx phy
  Ptr<MmWaveSidelinkPhy> rx_phy = CreateObject<MmWaveSidelinkPhy> (rx_ssp, pmc);

  // creating mac instance in order to be able to call the method DoSlotIndication from MmWaveSidelinkPhy::StartSlot()
  Ptr<MmWaveSidelinkMac> rx_mac = CreateObject<MmWaveSidelinkMac> (pmc);
  rx_mac->SetRnti(2);
  rx_phy->SetPhySapUser(rx_mac->GetPhySapUser());

  // connect the rx callback to the sink
  rx_ssp->SetPhyRxDataEndOkCallback (MakeCallback (&MmWaveVehicularSpectrumPhyTestCase1::Rx, this));

  // create and configure the chunk processor
  Ptr<mmwave::mmWaveChunkProcessor> pData = Create<mmwave::mmWaveChunkProcessor> ();
  pData->AddCallback (MakeCallback (&MmWaveSidelinkSpectrumPhy::UpdateSinrPerceived, rx_ssp));
  pData->AddCallback (MakeCallback (&MmWaveVehicularSpectrumPhyTestCase1::UpdateSinrPerceived, this));
  rx_ssp->AddDataSinrChunkProcessor (pData);

  // create the tx and rx devices
  Ptr<NetDevice> txDev = CreateObject<MmWaveVehicularNetDevice> (tx_phy, tx_mac);
  Ptr<NetDevice> rxDev = CreateObject<MmWaveVehicularNetDevice> (rx_phy, rx_mac);
  n.Get (0)->AddDevice (txDev);
  n.Get (1)->AddDevice (rxDev);
  txDev->SetNode (n.Get (0));
  rxDev->SetNode (n.Get (1));
  tx_ssp->SetDevice (txDev);
  rx_ssp->SetDevice (rxDev);

  // intialize the tx and rx phy
  tx_phy->AddDevice (2, rxDev);
  rx_phy->AddDevice (1, txDev);

  Simulator::Schedule (MicroSeconds (1000), &MmWaveVehicularSpectrumPhyTestCase1::Tx, this, tx_phy, ipi);

  Simulator::Stop (Seconds (2));
  Simulator::Run ();
  Simulator::Destroy ();

  // compute the metrics
  double average_sinr = 0;
  for (auto s : m_sinr)
  {
    average_sinr += s;
  }
  average_sinr /= m_sinr.size ();

  NS_LOG_UNCOND ("distance " << distance << " average SINR " << average_sinr << " PRR " << double (m_rx) / double (m_tx));
  //NS_LOG_UNCOND (distance << " " << average_sinr << " " << double (m_rx) / double (m_tx));
}

/**
 * Test suite for the class MmWaveSidelinkPhy
 */
class MmWaveVehicularSpectrumPhyTestSuite : public TestSuite
{
public:
  MmWaveVehicularSpectrumPhyTestSuite ();
};

MmWaveVehicularSpectrumPhyTestSuite::MmWaveVehicularSpectrumPhyTestSuite ()
  : TestSuite ("mmwave-vehicular-sidelink-phy-test", UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new MmWaveVehicularSpectrumPhyTestCase1, TestCase::QUICK);
}

static MmWaveVehicularSpectrumPhyTestSuite MmWaveVehicularSpectrumPhyTestSuite;
