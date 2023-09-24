#include "mros2.h"
#include "mros2-platform.h"
/* Twist型を使用するためのヘッダファイル */
#include "geometry_msgs/msg/twist.hpp"

#include "pico_msgs/msg/light_sensor.hpp"

/* C実装の pico_driver をC++モジュールからリンク */
extern "C"
{
#include "pico.h"
}

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t PubTaskHandle = NULL;

mros2::Node node;

void PubTask(void *arg)
{
  mros2::Publisher pub = node.create_publisher<pico_msgs::msg::LightSensor>("pico_light_sensor", 10);

  MROS2_INFO("ready to publish message");

  while (1)
  {
    pico_msgs::msg::LightSensor msg;

    msg.right = read_light_sensor(R);
    msg.forward_r = read_light_sensor(FR);
    msg.forward_l = read_light_sensor(FL);
    msg.left = read_light_sensor(L);

    pub.publish(msg);
    MROS2_INFO("publishing LightSensor msg! R = %d, FR = %d, FL = %d, L = %d",
               msg.right, msg.forward_r, msg.forward_l, msg.left);

    delay_ms(1000);
  }
}

/* /cmd_vel の購読時に実行されるコールバック関数 */
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
  node = mros2::Node::create_node("pico_node");
  /* 購読者として登録・トピックの型と名前，コールバック関数の設定 */
  mros2::Subscriber sub = node.create_subscription<geometry_msgs::msg::Twist>("cmd_vel", 10, twist_callback);

  MROS2_INFO("ready to subscribe message");

  xTaskCreate(PubTask, "PubTask", 4096, NULL, 10, &PubTaskHandle);

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
