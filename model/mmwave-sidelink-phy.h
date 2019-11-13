/*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
*   along with this program; if not, write to the Free Software:100cento

*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/


#ifndef SRC_MMWAVE_SIDELINK_PHY_H_
#define SRC_MMWAVE_SIDELINK_PHY_H_

#include "mmwave-sidelink-spectrum-phy.h"

namespace ns3 {

namespace mmwave {

class MmWaveSidelinkPhy : public Object
{

public:

  /**
   * Dummy constructor, it is not used
   */
  MmWaveSidelinkPhy ();

  /**
   * MmWaveSidelinkPhy real constructor
   * \param channelPhy spectrum phy
   * \param confParams instance of MmWavePhyMacCommon containing the
   *        configuration parameters
   *
   * Usually called by the helper. It starts the event loop for the device.
   */
  MmWaveSidelinkPhy (Ptr<MmWaveSidelinkSpectrumPhy> spectrumPhy, Ptr<MmWavePhyMacCommon> confParams);

  /**
   * Desctructor
   */
  virtual ~MmWaveSidelinkPhy ();

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

  /**
   * Set the tx power
   * \param the tx power in dBm
   */
  void SetTxPower (double power);

  /**
   * Returns the tx power
   * \return the tx power in dBm
   */
  double GetTxPower () const;

  /**
   * Set the noise figure
   * \param the noise figure in dB
   */
  void SetNoiseFigure (double pf);

  /**
   * Returns the noise figure
   * \return the noise figure in dB
   */
  double GetNoiseFigure () const;

  /**
   * Returns the MmWavePhyMacCommon instance associated with this phy containing
   * the configuration parameters
   * \return the MmWavePhyMacCommon instance
   */
  Ptr<MmWavePhyMacCommon> GetConfigurationParameters (void) const;

  /**
   * Returns the SpectrumPhy instance associated with this phy
   * \return the SpectrumPhy instance
   */
  Ptr<MmWaveSidelinkSpectrumPhy> GetSpectrumPhy () const;

  /**
   * Add a transport block to the transmission buffer, which will be sent in the
   * current slot.
   * \param pb the packet burst containing the packets to be sent
   * \param info the SlotAllocInfo instance containg the transmission information
   */
  void AddTransportBlock (Ptr<PacketBurst> pb, SlotAllocInfo info);

private:

  /**
   * Start a slot. Send all the transport blocks in the buffer.
   * \param slotNum the slot index
   */
  void StartSlot (uint8_t slotNum);

  /**
   * Transmit a transport block
   * \param pb the packet burst containing the packets to be sent
   * \param info the SlotAllocInfo instance containg the transmission information
   * \return the number of symbols used to send this TB
   */
  uint8_t SlData (Ptr<PacketBurst> pb, SlotAllocInfo info);

  /**
   * Set the transmission mask and creates the power spectral density for the
   * transmission
   * \return mask indicating the suchannels used for the transmission
   */
  std::vector<int> SetSubChannelsForTransmission ();

  /**
   * Send the packet burts
   * \param pb the packet burst
   * \param duration the duration of the transmissin
   * \param slotInd the slot index
   * \param mcs the MCS value
   * \param size the size of the transport block to send
   * \param rbBitmap the mask indicating the suchannels to be used for the
            transmission
   */
  void SendDataChannels (Ptr<PacketBurst> pb, Time duration, uint8_t slotInd, uint8_t mcs, uint32_t size, std::vector<int> rbBitmap);

private:
  double m_txPower; //!< the transmission power in dBm
  double m_noiseFigure; //!< the noise figure in dB
  Ptr<MmWaveSidelinkSpectrumPhy> m_sidelinkSpectrumPhy; //!< the SpectrumPhy instance associated with this PHY
  Ptr<MmWavePhyMacCommon> m_phyMacConfig; //!< the configuration parameters
  typedef std::pair<Ptr<PacketBurst>, SlotAllocInfo> PhyBufferEntry; //!< type of the phy buffer entries
  std::list<PhyBufferEntry> m_phyBuffer; //!< buffer of transport blocks to send in the current slot
};

} // namespace mmwave
} // namespace ns3

#endif /* SRC_MMWAVE_SIDELINK_PHY_H_ */
