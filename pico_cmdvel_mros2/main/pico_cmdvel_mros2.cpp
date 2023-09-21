#include "mros2.h"
#include "mros2-platform.h"
#include "geometry_msgs/msg/twist.hpp"

/* C実装の pico_driver をC++モジュールからリンク */
extern "C" {
#include "pico.h"
}

/* /cmd_vel を購読時に実行されるコールバック関数 */
void twist_callback(geometry_msgs::msg::Twist *msg)
{
  MROS2_INFO("subscribed Twist msg! linear = %.2f, angular = %.2f",
             msg->linear.x, msg->angular.z);

  /* 購読したTwist値に合わせてモータを駆動させる */
  drive_by_cmd_vel(msg->linear.x, msg->angular.z);
}

extern "C" void app_main(void)
{
  /* ネットワーク接続とPi:Coドライバの初期化 */
  mros2_platform_network_connect();
  pico_init();

  MROS2_INFO("pico_cmdvel_mros2 start!");

  /* mros2オブジェクトの初期化 */
  mros2::init(0, NULL);
  /* mros2ノードの生成 */
  mros2::Node node = mros2::Node::create_node("sub_twist");
  /* 購読対象のトピックの型・名前・コールバック関数の設定 */
  mros2::Subscriber sub = node.create_subscription<geometry_msgs::msg::Twist>("cmd_vel", 10, userCallback);

  MROS2_INFO("ready to subscribe message");

  /* プッシュスイッチでモータ駆動の制御：青で開始／赤で停止 */
  while (1)
  {
    if (!read_switch(SW_R))
    {
      motor_start();
    }
    else if (!read_switch(SW_L))
    {
      motor_stop();
    }
    delay_ms(10);
  }

  mros2::spin();

  return;
}
