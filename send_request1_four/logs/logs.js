Page({
  data: {
    videoUrl: 'http://192.168.1.106/mjpeg/1', // 初始视频URL
    inputUrl: '', // 用户输入的URL
    currentAngle1: 90, // 舵机1初始角度
    currentAngle2: 90, // 舵机2初始角度
    currentAngle3: 90, // 舵机3初始角度
    currentAngle4: 90, // 舵机4初始角度
    intervalId: null, // 用于存储定时器ID
    videoVisible: true // 初始显示视频
  },
  onUrlInput: function (e) {
    this.setData({
      inputUrl: e.detail.value
    });
  },
  showVideo: function () {
    if (this.data.inputUrl) {
      this.setData({
        videoUrl: this.data.inputUrl, // 使用用户输入的URL
      });
    }
    this.setData({
      videoVisible: true // 显示视频
    });
  },
  decreaseAngle1: function () {
    this.changeAngle('currentAngle1', -5); // 点按减少5度
  },
  increaseAngle1: function () {
    this.changeAngle('currentAngle1', 5); // 点按增加5度
  },
  resetAngle1: function () {
    this.setData({ currentAngle1: 90 });
    this.sendCommand(1, 90);
  },
  decreaseAngle2: function () {
    this.changeAngle('currentAngle2', -5); // 点按减少5度
  },
  increaseAngle2: function () {
    this.changeAngle('currentAngle2', 5); // 点按增加5度
  },
  resetAngle2: function () {
    this.setData({ currentAngle2: 90 });
    this.sendCommand(2, 90);
  },
  decreaseAngle3: function () {
    this.changeAngle('currentAngle3', -5); // 点按减少5度
  },
  increaseAngle3: function () {
    this.changeAngle('currentAngle3', 5); // 点按增加5度
  },
  resetAngle3: function () {
    this.setData({ currentAngle3: 90 });
    this.sendCommand(3, 90);
  },
  decreaseAngle4: function () {
    this.changeAngle('currentAngle4', -5); // 点按减少5度
  },
  increaseAngle4: function () {
    this.changeAngle('currentAngle4', 5); // 点按增加5度
  },
  resetAngle4: function () {
    this.setData({ currentAngle4: 90 });
    this.sendCommand(4, 90);
  },
  resetAllAngles: function () {
    this.resetAngle1();
    this.resetAngle2();
    this.resetAngle3();
    this.resetAngle4();
  },
  changeAngle: function (angleKey, delta) {
    let newAngle = this.data[angleKey] + delta;
    if (newAngle < 0) newAngle = 0;
    if (newAngle > 180) newAngle = 180;
    this.setData({ [angleKey]: newAngle });
    const servoNumber = angleKey === 'currentAngle1' ? 1 : angleKey === 'currentAngle2' ? 2 : angleKey === 'currentAngle3' ? 3 : 4;
    this.sendCommand(servoNumber, newAngle);
  },
  startDecreaseAngle1: function () {
    this.startAngleChange('currentAngle1', -10); // 长按减少10度
  },
  startIncreaseAngle1: function () {
    this.startAngleChange('currentAngle1', 10); // 长按增加10度
  },
  startDecreaseAngle2: function () {
    this.startAngleChange('currentAngle2', -10); // 长按减少10度
  },
  startIncreaseAngle2: function () {
    this.startAngleChange('currentAngle2', 10); // 长按增加10度
  },
  startDecreaseAngle3: function () {
    this.startAngleChange('currentAngle3', -10); // 长按减少10度
  },
  startIncreaseAngle3: function () {
    this.startAngleChange('currentAngle3', 10); // 长按增加10度
  },
  startDecreaseAngle4: function () {
    this.startAngleChange('currentAngle4', -10); // 长按减少10度
  },
  startIncreaseAngle4: function () {
    this.startAngleChange('currentAngle4', 10); // 长按增加10度
  },
  startAngleChange: function (angleKey, delta) {
    this.stopAngleChange(); // 确保没有其他定时器在运行
    this.data.intervalId = setInterval(() => {
      this.changeAngle(angleKey, delta);
    }, 200); // 每200毫秒改变一次角度
  },
  stopAngleChange: function () {
    if (this.data.intervalId) {
      clearInterval(this.data.intervalId);
      this.data.intervalId = null;
    }
  },
  sendCommand: function (servoNumber, angle) {
    const angleKey = `angle_${servoNumber}`;
    wx.request({
      url: 'https://iot-api.heclouds.com/thingmodel/set-device-desired-property',
      method: 'POST',
      header: {
        "Content-Type": "application/json",
        "authorization": "version=2018-10-31&res=products%2F4lF0z4M2J1%2Fdevices%2Ftest&et=1744387714&method=md5&sign=%2BFLMW9brZHqbg7afprcbUw%3D%3D"
      },
      data: {
        "product_id": "4lF0z4M2J1",
        "device_name": "test",
        "params": {
          [angleKey]: angle // 发送当前角度
        }
      },
      success(res) {
        console.log("Command sent successfully:", res);
      },
      fail(err) {
        console.error("Failed to send command:", err);
      }
    });
  }
});
