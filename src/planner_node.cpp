#include <chrono>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/path.hpp"
#include "nav2_msgs/action/compute_path_to_pose.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using ComputePathToPose = nav2_msgs::action::ComputePathToPose;
using GoalHandleComputePathToPose = rclcpp_action::ClientGoalHandle<ComputePathToPose>;

class PlannerNode : public rclcpp::Node
{
public:
  PlannerNode() : Node("planner_node")
  {
    publisher_ = this->create_publisher<nav_msgs::msg::Path>("/path", 10);

    action_client_ = rclcpp_action::create_client<ComputePathToPose>(
        this, "/compute_path_to_pose");

    // Wait for the action server to be available
    while (!action_client_->wait_for_action_server(std::chrono::seconds(1))) {
      RCLCPP_INFO(this->get_logger(), "Waiting for /compute_path_to_pose action server...");
    }

    send_goal();
  }

private:
  rclcpp_action::Client<ComputePathToPose>::SharedPtr action_client_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr publisher_;

  void send_goal()
  {
    auto goal_msg = ComputePathToPose::Goal();

    // Set the start pose (0,0)
    // geometry_msgs::msg::PoseStamped start;
    // start.header.frame_id = "map";
    // start.header.stamp = this->now();
    // start.pose.position.x = 0.0;
    // start.pose.position.y = 0.0;
    // start.pose.position.z = 0.0;
    // start.pose.orientation.w = 1.0;
    // goal_msg.start = start;

    // Set the goal pose (0,0)
    geometry_msgs::msg::PoseStamped goal;
    goal.header.frame_id = "map";
    goal.header.stamp = this->now();
    goal.pose.position.x = 0.0;
    goal.pose.position.y = 0.0;
    goal.pose.position.z = 0.0;
    goal.pose.orientation.w = 1.0;
    goal_msg.goal = goal;

    // Send goal
    auto send_goal_options = rclcpp_action::Client<ComputePathToPose>::SendGoalOptions();
    send_goal_options.result_callback =
        [this](const GoalHandleComputePathToPose::WrappedResult & result) {
          if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
            RCLCPP_INFO(this->get_logger(), "Received path, publishing to /path");
            publisher_->publish(result.result->path);
          } else {
            RCLCPP_ERROR(this->get_logger(), "Failed to get path");
          }
        };

    action_client_->async_send_goal(goal_msg, send_goal_options);
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<PlannerNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}

