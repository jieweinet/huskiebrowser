// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/services/bluetooth_config/device_cache_impl.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "base/strings/strcat.h"
#include "base/strings/string_number_conversions.h"
#include "base/test/task_environment.h"
#include "chromeos/services/bluetooth_config/fake_adapter_state_controller.h"
#include "chromeos/services/bluetooth_config/fake_device_name_manager.h"
#include "device/bluetooth/bluetooth_common.h"
#include "device/bluetooth/test/mock_bluetooth_adapter.h"
#include "device/bluetooth/test/mock_bluetooth_device.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chromeos {
namespace bluetooth_config {
namespace {

using PairedDeviceList = std::vector<mojom::PairedBluetoothDevicePropertiesPtr>;
using UnpairedDeviceList = std::vector<mojom::BluetoothDevicePropertiesPtr>;
using NiceMockDevice =
    std::unique_ptr<testing::NiceMock<device::MockBluetoothDevice>>;

const uint32_t kTestBluetoothClass = 1337u;
const char kTestBluetoothName[] = "testName";
const char kTestBluetoothNickname[] = "nickname";

class FakeObserver : public DeviceCache::Observer {
 public:
  FakeObserver() = default;
  ~FakeObserver() override = default;

  size_t num_paired_list_changed_calls() const {
    return num_paired_list_changed_calls_;
  }

  size_t num_unpaired_list_changed_calls() const {
    return num_unpaired_list_changed_calls_;
  }

 private:
  // DeviceCache::Observer:
  void OnPairedDevicesListChanged() override {
    ++num_paired_list_changed_calls_;
  }

  void OnUnpairedDevicesListChanged() override {
    ++num_unpaired_list_changed_calls_;
  }

  size_t num_paired_list_changed_calls_ = 0u;

  size_t num_unpaired_list_changed_calls_ = 0u;
};

}  // namespace

class DeviceCacheImplTest : public testing::Test {
 protected:
  DeviceCacheImplTest() = default;
  DeviceCacheImplTest(const DeviceCacheImplTest&) = delete;
  DeviceCacheImplTest& operator=(const DeviceCacheImplTest&) = delete;
  ~DeviceCacheImplTest() override = default;

  // testing::Test:
  void SetUp() override {
    mock_adapter_ =
        base::MakeRefCounted<testing::NiceMock<device::MockBluetoothAdapter>>();
    ON_CALL(*mock_adapter_, GetDevices())
        .WillByDefault(
            testing::Invoke(this, &DeviceCacheImplTest::GenerateDevices));
  }

  void TearDown() override { device_cache_->RemoveObserver(&fake_observer_); }

  void Init() {
    device_cache_ = std::make_unique<DeviceCacheImpl>(
        &fake_adapter_state_controller_, mock_adapter_,
        &fake_device_name_manager_);
    device_cache_->AddObserver(&fake_observer_);
  }

  void SetBluetoothSystemState(mojom::BluetoothSystemState system_state) {
    fake_adapter_state_controller_.SetSystemState(system_state);
  }

  void AddDevice(bool paired,
                 bool connected,
                 std::string* id_out,
                 const absl::optional<int8_t> inquiry_rssi = absl::nullopt) {
    // We use the number of devices created in this test as the address.
    std::string address = base::NumberToString(num_devices_created_);
    ++num_devices_created_;

    // Mock devices have their ID set to "${address}-Identifier".
    *id_out = base::StrCat({address, "-Identifier"});

    auto mock_device =
        std::make_unique<testing::NiceMock<device::MockBluetoothDevice>>(
            mock_adapter_.get(), kTestBluetoothClass, kTestBluetoothName,
            address, paired, connected);
    ON_CALL(*mock_device, GetInquiryRSSI())
        .WillByDefault(testing::Return(inquiry_rssi));

    device::BluetoothDevice* device = mock_device.get();
    mock_devices_.push_back(std::move(mock_device));

    if (device_cache_)
      device_cache_->DeviceAdded(mock_adapter_.get(), device);
  }

  void RemoveDevice(const std::string& device_id) {
    auto it = FindDevice(device_id);
    EXPECT_TRUE(it != mock_devices_.end());

    NiceMockDevice device = std::move(*it);
    mock_devices_.erase(it);

    device_cache_->DeviceRemoved(mock_adapter_.get(), device.get());
  }

  void ChangePairingState(const std::string& device_id, bool is_now_paired) {
    auto it = FindDevice(device_id);
    EXPECT_TRUE(it != mock_devices_.end());

    ON_CALL(**it, IsPaired()).WillByDefault(testing::Return(is_now_paired));

    device_cache_->DevicePairedChanged(mock_adapter_.get(), it->get(),
                                       is_now_paired);
  }

  void ChangeDeviceType(const std::string& device_id,
                        device::BluetoothDeviceType new_type) {
    auto it = FindDevice(device_id);
    EXPECT_TRUE(it != mock_devices_.end());

    ON_CALL(**it, GetDeviceType()).WillByDefault(testing::Return(new_type));

    device_cache_->DeviceChanged(mock_adapter_.get(), it->get());
  }

  void ChangeInquiryRssi(const std::string& device_id,
                         const absl::optional<int8_t> inquiry_rssi) {
    std::vector<NiceMockDevice>::iterator it = FindDevice(device_id);
    EXPECT_TRUE(it != mock_devices_.end());

    ON_CALL(**it, GetInquiryRSSI())
        .WillByDefault(testing::Return(inquiry_rssi));

    device_cache_->DeviceChanged(mock_adapter_.get(), it->get());
  }

  void SetDeviceNickname(const std::string& device_id,
                         const std::string& nickname) {
    fake_device_name_manager_.SetDeviceNickname(device_id, nickname);
  }

  PairedDeviceList GetPairedDevices() {
    return device_cache_->GetPairedDevices();
  }

  UnpairedDeviceList GetUnpairedDevices() {
    return device_cache_->GetUnpairedDevices();
  }

  size_t GetNumPairedDeviceListObserverEvents() const {
    return fake_observer_.num_paired_list_changed_calls();
  }

  size_t GetNumUnpairedDeviceListObserverEvents() const {
    return fake_observer_.num_unpaired_list_changed_calls();
  }

 private:
  std::vector<const device::BluetoothDevice*> GenerateDevices() {
    std::vector<const device::BluetoothDevice*> devices;
    for (auto& device : mock_devices_)
      devices.push_back(device.get());
    return devices;
  }

  std::vector<NiceMockDevice>::iterator FindDevice(
      const std::string& device_id) {
    return std::find_if(mock_devices_.begin(), mock_devices_.end(),
                        [&device_id](const NiceMockDevice& device) {
                          return device_id == device->GetIdentifier();
                        });
  }

  base::test::TaskEnvironment task_environment_;

  std::vector<NiceMockDevice> mock_devices_;
  size_t num_devices_created_ = 0u;

  FakeAdapterStateController fake_adapter_state_controller_;
  FakeDeviceNameManager fake_device_name_manager_;
  scoped_refptr<testing::NiceMock<device::MockBluetoothAdapter>> mock_adapter_;
  FakeObserver fake_observer_;

  std::unique_ptr<DeviceCacheImpl> device_cache_;
};

TEST_F(DeviceCacheImplTest, AddAndRemovePairedDevices) {
  Init();
  EXPECT_TRUE(GetPairedDevices().empty());

  // Add device 1 (disconnected).
  std::string paired_device_id1;
  AddDevice(/*paired=*/true, /*connected=*/false, &paired_device_id1);
  EXPECT_EQ(1u, GetNumPairedDeviceListObserverEvents());
  PairedDeviceList list = GetPairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(paired_device_id1, list[0]->device_properties->id);

  // Add device 2 (connected). Paired connected devices should be returned
  // before disconnected ones.
  std::string paired_device_id2;
  AddDevice(/*paired=*/true, /*connected=*/true, &paired_device_id2);
  EXPECT_EQ(2u, GetNumPairedDeviceListObserverEvents());
  list = GetPairedDevices();
  EXPECT_EQ(2u, list.size());
  EXPECT_EQ(paired_device_id2, list[0]->device_properties->id);
  EXPECT_EQ(paired_device_id1, list[1]->device_properties->id);

  // Remove device 2; only device 1 should be returned.
  RemoveDevice(paired_device_id2);
  EXPECT_EQ(3u, GetNumPairedDeviceListObserverEvents());
  list = GetPairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(paired_device_id1, list[0]->device_properties->id);
}

TEST_F(DeviceCacheImplTest, AddAndRemoveUnpairedDevices) {
  Init();
  EXPECT_TRUE(GetUnpairedDevices().empty());

  // Add device 1.
  std::string unpaired_device_id1;
  AddDevice(/*paired=*/false, /*connected=*/false, &unpaired_device_id1,
            /*inquiry_rssi=*/1);
  EXPECT_EQ(1u, GetNumUnpairedDeviceListObserverEvents());
  UnpairedDeviceList list = GetUnpairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(unpaired_device_id1, list[0]->id);

  // Add device 2 with higher signal strength than device 1. Device 2 should be
  // returned first.
  std::string unpaired_device_id2;
  AddDevice(/*paired=*/false, /*connected=*/false, &unpaired_device_id2,
            /*inquiry_rssi=*/2);
  EXPECT_EQ(2u, GetNumUnpairedDeviceListObserverEvents());
  list = GetUnpairedDevices();
  EXPECT_EQ(2u, list.size());
  EXPECT_EQ(unpaired_device_id2, list[0]->id);
  EXPECT_EQ(unpaired_device_id1, list[1]->id);

  // Remove device 2; only device 1 should be returned.
  RemoveDevice(unpaired_device_id2);
  EXPECT_EQ(3u, GetNumUnpairedDeviceListObserverEvents());
  list = GetUnpairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(unpaired_device_id1, list[0]->id);
}

TEST_F(DeviceCacheImplTest, AddBeforeInit) {
  // Add device 1, a paired device, and device 2, an unpaired device before
  // initializing the class.
  std::string paired_device_id1;
  AddDevice(/*paired=*/true, /*connected=*/true, &paired_device_id1);
  std::string unpaired_device_id2;
  AddDevice(/*paired=*/false, /*connected=*/true, &unpaired_device_id2);
  Init();

  // Device 1 should be available in the paired device list from the getgo,
  // device 2 should not be present.
  PairedDeviceList paired_list = GetPairedDevices();
  EXPECT_EQ(1u, paired_list.size());
  EXPECT_EQ(paired_device_id1, paired_list[0]->device_properties->id);

  // Device 2 should be available in the unpaired device list from the getgo,
  // device 1 should not be present.
  UnpairedDeviceList unpaired_list = GetUnpairedDevices();
  EXPECT_EQ(1u, unpaired_list.size());
  EXPECT_EQ(unpaired_device_id2, unpaired_list[0]->id);

  // Add paired device 3 and verify that device 1 and device 3 are returned in
  // the paired device list.
  std::string paired_device_id3;
  AddDevice(/*paired=*/true, /*connected=*/true, &paired_device_id3);
  EXPECT_EQ(1u, GetNumPairedDeviceListObserverEvents());
  paired_list = GetPairedDevices();
  EXPECT_EQ(2u, paired_list.size());
  EXPECT_EQ(paired_device_id1, paired_list[0]->device_properties->id);
  EXPECT_EQ(paired_device_id3, paired_list[1]->device_properties->id);

  // Add unpaired device 4 and verify that device 2 and device 4 are returned in
  // the unpaired device list.
  std::string unpaired_device_id4;
  AddDevice(/*paired=*/false, /*connected=*/true, &unpaired_device_id4);
  EXPECT_EQ(1u, GetNumUnpairedDeviceListObserverEvents());
  unpaired_list = GetUnpairedDevices();
  EXPECT_EQ(2u, unpaired_list.size());
  EXPECT_EQ(unpaired_device_id2, unpaired_list[0]->id);
  EXPECT_EQ(unpaired_device_id4, unpaired_list[1]->id);
}

TEST_F(DeviceCacheImplTest, AddPairedAndUnpairedDevices) {
  Init();
  EXPECT_TRUE(GetPairedDevices().empty());
  EXPECT_TRUE(GetUnpairedDevices().empty());

  // Add a paired device.
  std::string paired_device_id;
  AddDevice(/*paired=*/true, /*connected=*/true, &paired_device_id);
  EXPECT_EQ(1u, GetNumPairedDeviceListObserverEvents());
  PairedDeviceList paired_list = GetPairedDevices();
  EXPECT_EQ(1u, paired_list.size());
  EXPECT_EQ(paired_device_id, paired_list[0]->device_properties->id);

  // Unpaired device observer should not be notified and unpaired device list
  // should still be empty.
  EXPECT_EQ(0u, GetNumUnpairedDeviceListObserverEvents());
  EXPECT_TRUE(GetUnpairedDevices().empty());

  // Add an unpaired device.
  std::string unpaired_device_id;
  AddDevice(/*paired=*/false, /*connected=*/true, &unpaired_device_id);
  EXPECT_EQ(1u, GetNumUnpairedDeviceListObserverEvents());
  UnpairedDeviceList unpaired_list = GetUnpairedDevices();
  EXPECT_EQ(1u, unpaired_list.size());
  EXPECT_EQ(unpaired_device_id, unpaired_list[0]->id);

  // Paired device observer should not be notified and paired device list should
  // still only have one element.
  EXPECT_EQ(1u, GetNumPairedDeviceListObserverEvents());
  EXPECT_EQ(1u, paired_list.size());
  EXPECT_EQ(paired_device_id, paired_list[0]->device_properties->id);
}

TEST_F(DeviceCacheImplTest, PairingStateChanges) {
  Init();
  EXPECT_TRUE(GetPairedDevices().empty());
  EXPECT_TRUE(GetUnpairedDevices().empty());

  // Add a paired device.
  std::string paired_device_id;
  AddDevice(/*paired=*/true, /*connected=*/true, &paired_device_id);
  EXPECT_EQ(1u, GetNumPairedDeviceListObserverEvents());
  EXPECT_EQ(0u, GetNumUnpairedDeviceListObserverEvents());
  PairedDeviceList list = GetPairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(paired_device_id, list[0]->device_properties->id);
  EXPECT_TRUE(GetUnpairedDevices().empty());

  // Unpair; the paired list observer should be notified and the device should
  // not be returned in the paired list.
  ChangePairingState(paired_device_id, /*is_now_paired=*/false);
  EXPECT_EQ(2u, GetNumPairedDeviceListObserverEvents());
  EXPECT_TRUE(GetPairedDevices().empty());

  // The unpaired list observer should also be notified and the device should be
  // returned in the unpaired list.
  EXPECT_EQ(1u, GetNumUnpairedDeviceListObserverEvents());
  UnpairedDeviceList unpaired_list = GetUnpairedDevices();
  EXPECT_EQ(1u, unpaired_list.size());
  EXPECT_EQ(paired_device_id, unpaired_list[0]->id);

  // Re-pair; the paired list observer should be notified, and the device should
  // be returned in the paired list.
  ChangePairingState(paired_device_id, /*is_now_paired=*/true);
  EXPECT_EQ(3u, GetNumPairedDeviceListObserverEvents());
  list = GetPairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(paired_device_id, list[0]->device_properties->id);

  // The unpaired list observer should also be notified and the device should
  // not be returned in the unpaired list.
  EXPECT_EQ(2u, GetNumUnpairedDeviceListObserverEvents());
  EXPECT_TRUE(GetUnpairedDevices().empty());
}

TEST_F(DeviceCacheImplTest, PairedDeviceNicknameChanges) {
  Init();
  EXPECT_TRUE(GetPairedDevices().empty());

  // Add a paired device.
  std::string paired_device_id;
  AddDevice(/*paired=*/true, /*connected=*/true, &paired_device_id);
  EXPECT_EQ(1u, GetNumPairedDeviceListObserverEvents());
  PairedDeviceList list = GetPairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(paired_device_id, list[0]->device_properties->id);
  EXPECT_FALSE(list[0]->nickname);

  // Set the device's nickname
  SetDeviceNickname(paired_device_id, kTestBluetoothNickname);
  EXPECT_EQ(2u, GetNumPairedDeviceListObserverEvents());
  list = GetPairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(paired_device_id, list[0]->device_properties->id);
  EXPECT_EQ(kTestBluetoothNickname, list[0]->nickname);
}

TEST_F(DeviceCacheImplTest, PairedDeviceBluetoothClassChanges) {
  Init();
  EXPECT_TRUE(GetPairedDevices().empty());

  // Add a paired device.
  std::string paired_device_id;
  AddDevice(/*paired=*/true, /*connected=*/true, &paired_device_id);
  EXPECT_EQ(1u, GetNumPairedDeviceListObserverEvents());
  PairedDeviceList list = GetPairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(paired_device_id, list[0]->device_properties->id);
  EXPECT_EQ(mojom::DeviceType::kUnknown,
            list[0]->device_properties->device_type);

  // Change its device type.
  ChangeDeviceType(paired_device_id, device::BluetoothDeviceType::PHONE);
  EXPECT_EQ(2u, GetNumPairedDeviceListObserverEvents());
  list = GetPairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(paired_device_id, list[0]->device_properties->id);
  EXPECT_EQ(mojom::DeviceType::kPhone, list[0]->device_properties->device_type);
}

TEST_F(DeviceCacheImplTest, UnpairedDeviceBluetoothClassChanges) {
  Init();
  EXPECT_TRUE(GetUnpairedDevices().empty());

  // Add a unpaired device.
  std::string unpaired_device_id;
  AddDevice(/*paired=*/false, /*connected=*/true, &unpaired_device_id);
  EXPECT_EQ(1u, GetNumUnpairedDeviceListObserverEvents());
  UnpairedDeviceList list = GetUnpairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(unpaired_device_id, list[0]->id);
  EXPECT_EQ(mojom::DeviceType::kUnknown, list[0]->device_type);

  // Change its device type.
  ChangeDeviceType(unpaired_device_id, device::BluetoothDeviceType::PHONE);
  EXPECT_EQ(2u, GetNumUnpairedDeviceListObserverEvents());
  list = GetUnpairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(unpaired_device_id, list[0]->id);
  EXPECT_EQ(mojom::DeviceType::kPhone, list[0]->device_type);
}

TEST_F(DeviceCacheImplTest, UnpairedDeviceSignalStrengthChanges) {
  Init();
  EXPECT_TRUE(GetUnpairedDevices().empty());

  // Add device 1.
  std::string unpaired_device_id1;
  AddDevice(/*paired=*/false, /*connected=*/false, &unpaired_device_id1,
            /*inquiry_rssi=*/1);
  EXPECT_EQ(1u, GetNumUnpairedDeviceListObserverEvents());
  UnpairedDeviceList list = GetUnpairedDevices();
  EXPECT_EQ(1u, list.size());
  EXPECT_EQ(unpaired_device_id1, list[0]->id);

  // Add device 2 with higher signal strength than device 1. Device 2 should be
  // returned first.
  std::string unpaired_device_id2;
  AddDevice(/*paired=*/false, /*connected=*/false, &unpaired_device_id2,
            /*inquiry_rssi=*/2);
  EXPECT_EQ(2u, GetNumUnpairedDeviceListObserverEvents());
  list = GetUnpairedDevices();
  EXPECT_EQ(2u, list.size());
  EXPECT_EQ(unpaired_device_id2, list[0]->id);
  EXPECT_EQ(unpaired_device_id1, list[1]->id);

  // Update device 1's signal strength to be greater than device 2. Device 1
  // should now be returned first.
  ChangeInquiryRssi(unpaired_device_id1, 3);
  EXPECT_EQ(3u, GetNumUnpairedDeviceListObserverEvents());
  list = GetUnpairedDevices();
  EXPECT_EQ(2u, list.size());
  EXPECT_EQ(unpaired_device_id1, list[0]->id);
  EXPECT_EQ(unpaired_device_id2, list[1]->id);
}

TEST_F(DeviceCacheImplTest, BluetoothTurnsOff) {
  Init();
  EXPECT_TRUE(GetPairedDevices().empty());
  EXPECT_TRUE(GetUnpairedDevices().empty());

  // Add a paired device and an unpaired device.
  std::string paired_device_id1;
  AddDevice(/*paired=*/true, /*connected=*/true, &paired_device_id1);
  std::string unpaired_device_id2;
  AddDevice(/*paired=*/false, /*connected=*/true, &unpaired_device_id2);

  // Both devices should be present in their respective lists.
  PairedDeviceList paired_list = GetPairedDevices();
  EXPECT_EQ(1u, GetNumPairedDeviceListObserverEvents());
  EXPECT_EQ(1u, paired_list.size());
  EXPECT_EQ(paired_device_id1, paired_list[0]->device_properties->id);
  UnpairedDeviceList unpaired_list = GetUnpairedDevices();
  EXPECT_EQ(1u, GetNumUnpairedDeviceListObserverEvents());
  EXPECT_EQ(1u, unpaired_list.size());
  EXPECT_EQ(unpaired_device_id2, unpaired_list[0]->id);

  // Turn off Bluetooth.
  SetBluetoothSystemState(mojom::BluetoothSystemState::kDisabling);

  // Observers for both lists should be notified and empty lists returned.
  paired_list = GetPairedDevices();
  EXPECT_EQ(2u, GetNumPairedDeviceListObserverEvents());
  EXPECT_TRUE(GetPairedDevices().empty());
  unpaired_list = GetUnpairedDevices();
  EXPECT_EQ(2u, GetNumUnpairedDeviceListObserverEvents());
  EXPECT_TRUE(GetUnpairedDevices().empty());
}

}  // namespace bluetooth_config
}  // namespace chromeos
