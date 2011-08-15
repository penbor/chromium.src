// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_CROS_NATIVE_NETWORK_PARSER_H_
#define CHROME_BROWSER_CHROMEOS_CROS_NATIVE_NETWORK_PARSER_H_
#pragma once

#include "chrome/browser/chromeos/cros/network_parser.h"
#include "base/compiler_specific.h"  // for OVERRIDE

namespace chromeos {

// This is the network device parser that parses the data from the
// network stack on the native platform.  Currently it parses
// FlimFlam-provided information.
class NativeNetworkDeviceParser : public NetworkDeviceParser {
 public:
  NativeNetworkDeviceParser();
  virtual ~NativeNetworkDeviceParser();
  virtual bool ParseValue(PropertyIndex index,
                          const Value& value,
                          NetworkDevice* device) OVERRIDE;
 protected:
  virtual ConnectionType ParseType(const std::string& type) OVERRIDE;

  // Parsing helper routines specific to native network devices.
  virtual bool ParseApnList(const ListValue& list, CellularApnList* apn_list);
  virtual bool ParseFoundNetworksFromList(const ListValue& list,
                                          CellularNetworkList* found_networks);
  virtual SimLockState ParseSimLockState(const std::string& state);
  virtual bool ParseSimLockStateFromDictionary(const DictionaryValue& info,
                                               SimLockState* out_state,
                                               int* out_retries);
  virtual TechnologyFamily ParseTechnologyFamily(
      const std::string& technology_family);

 private:
  DISALLOW_COPY_AND_ASSIGN(NativeNetworkDeviceParser);
};

// This is the network parser that parses the data from the network
// stack on the native platform.  Currently it parses
// FlimFlam-provided information.
class NativeNetworkParser : public NetworkParser {
 public:
  NativeNetworkParser();
  virtual ~NativeNetworkParser();
  static const EnumMapper<PropertyIndex>* property_mapper();
  static const ConnectionType ParseConnectionType(const std::string& type);
 protected:
  virtual bool ParseValue(PropertyIndex index,
                          const Value& value,
                          Network* network) OVERRIDE;
  virtual ConnectionType ParseType(const std::string& type) OVERRIDE;
  virtual ConnectionType ParseTypeFromDictionary(
      const DictionaryValue& info) OVERRIDE;
  virtual ConnectionMode ParseMode(const std::string& mode) OVERRIDE;
  virtual ConnectionState ParseState(const std::string& state) OVERRIDE;
  virtual ConnectionError ParseError(const std::string& error) OVERRIDE;
 private:
  DISALLOW_COPY_AND_ASSIGN(NativeNetworkParser);
};

// Below are specific types of network parsers.
class NativeEthernetNetworkParser : public NativeNetworkParser {
 public:
  NativeEthernetNetworkParser();
  virtual ~NativeEthernetNetworkParser();
 private:
  // NOTE: Uses base class ParseValue, etc.

  DISALLOW_COPY_AND_ASSIGN(NativeEthernetNetworkParser);
};

// Base for wireless networks.
class NativeWirelessNetworkParser : public NativeNetworkParser {
 public:
  NativeWirelessNetworkParser();
  virtual ~NativeWirelessNetworkParser();
  virtual bool ParseValue(PropertyIndex index,
                          const Value& value,
                          Network* network) OVERRIDE;
 private:
  DISALLOW_COPY_AND_ASSIGN(NativeWirelessNetworkParser);
};

class NativeWifiNetworkParser : public NativeWirelessNetworkParser {
 public:
  NativeWifiNetworkParser();
  virtual ~NativeWifiNetworkParser();
  virtual bool ParseValue(PropertyIndex index,
                          const Value& value,
                          Network* network) OVERRIDE;
 protected:
  ConnectionSecurity ParseSecurity(const std::string& security);
  EAPMethod ParseEAPMethod(const std::string& method);
  EAPPhase2Auth ParseEAPPhase2Auth(const std::string& auth);
 private:
  DISALLOW_COPY_AND_ASSIGN(NativeWifiNetworkParser);
};

class NativeCellularNetworkParser : public NativeWirelessNetworkParser {
 public:
  NativeCellularNetworkParser();
  virtual ~NativeCellularNetworkParser();
  virtual bool ParseValue(PropertyIndex index,
                          const Value& value,
                          Network* network) OVERRIDE;
 protected:
  ActivationState ParseActivationState(const std::string& state);
  NetworkTechnology ParseNetworkTechnology(
      const std::string& technology);
  NetworkRoamingState ParseRoamingState(
      const std::string& roaming_state);
 private:
  DISALLOW_COPY_AND_ASSIGN(NativeCellularNetworkParser);
};

class NativeVirtualNetworkParser : public NativeNetworkParser {
 public:
  NativeVirtualNetworkParser();
  virtual ~NativeVirtualNetworkParser();
  virtual bool ParseValue(PropertyIndex index,
                          const Value& value,
                          Network* network) OVERRIDE;
  virtual bool UpdateNetworkFromInfo(const DictionaryValue& info,
                                     Network* network) OVERRIDE;
 protected:
  bool ParseProviderValue(PropertyIndex index,
                                  const Value& value,
                                  VirtualNetwork* network);
  ProviderType ParseProviderType(const std::string& type);
 private:
  DISALLOW_COPY_AND_ASSIGN(NativeVirtualNetworkParser);
};


}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_CROS_NATIVE_NETWORK_PARSER_H_
