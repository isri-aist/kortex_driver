#pragma once

#include <robot_interface/RobotDriverTemplate.h>
#include <robot_interface/driver/api.h>

#include <ActuatorConfigClientRpc.h>
#include <BaseClientRpc.h>
#include <BaseCyclicClientRpc.h>
#include <RouterClient.h>
#include <SessionManager.h>
#include <TransportClientTcp.h>
#include <TransportClientUdp.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <chrono>

namespace k_api = Kinova::Api;

namespace kortex_driver
{

class RobotDriverKortex final : public mc_robot_interface::RobotDriver
{
public:
  RobotDriverKortex(const std::string & ip, uint16_t port = 10000);

  ~RobotDriverKortex() override;

  void sync() override;

  void setDataRead() override {}

  std::vector<double> getActualQ() override;
  std::vector<double> getActualQd() override;
  std::vector<double> getJointTorques() override;

  void servoJ(const std::vector<double> & q) override;
  void speedJ(const std::vector<double> & velocity) override;
  void tauJ(const std::vector<double> & torque) override;

  // Enable / disable freedrive mode.
  bool freeDrive(bool enable);

private:
  void connect();
  void disconnect() noexcept;
  void initializeCyclicCommand();

  void validateCommandSize(const std::vector<double> & command) const;

private:
  using Clock = std::chrono::steady_clock;
  Clock::time_point next_cycle_;
  bool cycle_timer_initialized_{false};

  static constexpr uint16_t default_tcp_port_ = 10000;
  static constexpr uint16_t default_udp_port_ = 10001;

  std::string ip_;
  uint16_t tcp_port_;
  uint16_t udp_port_ = default_udp_port_;

  std::unique_ptr<k_api::TransportClientTcp> tcp_transport_;

  std::unique_ptr<k_api::RouterClient> tcp_router_;

  std::unique_ptr<k_api::SessionManager> tcp_session_;

  std::unique_ptr<k_api::TransportClientUdp> udp_transport_;

  std::unique_ptr<k_api::RouterClient> udp_router_;

  std::unique_ptr<k_api::SessionManager> udp_session_;

  std::unique_ptr<k_api::Base::BaseClient> base_;

  std::unique_ptr<k_api::BaseCyclic::BaseCyclicClient> base_cyclic_;

  std::unique_ptr<k_api::ActuatorConfig::ActuatorConfigClient> actuator_config_;

  k_api::BaseCyclic::Feedback feedback_;
  k_api::BaseCyclic::Command command_;

  std::size_t actuator_count_ = 0;
  bool command_initialized_ = false;
};

} // namespace kortex_driver

extern "C"
{

  MC_ROBOT_DRIVER_DLLAPI void MC_RTC_ROBOT_DRIVER(std::vector<std::string> & classes);

  MC_ROBOT_DRIVER_DLLAPI
  mc_robot_interface::RobotDriver * create(const std::string & name, const std::string & ip, const uint16_t & port);

  MC_ROBOT_DRIVER_DLLAPI void destroy(mc_robot_interface::RobotDriver * driver);
}
