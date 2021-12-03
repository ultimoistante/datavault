var deviceIpAddress = "192.168.1.70";
// var deviceIpAddress = null;

ReconnectingWebSocket.prototype.sendObj = function (obj) {
    if (obj && typeof obj === 'object') {
        this.send(JSON.stringify(obj));
    }
}

var app = new Vue({
    el: '#app',
    data: function () {
        return {
            currentDateTime: '--:--:--',
            sbcPowered: false,
            sbcPowerStatusString: "???",
            sbcPowerButtonText: 'waiting...',
            gotSbcStatus: false,
            gotDateTime: false,
            activationTime: {
                hour: 0,
                minute: 0,
                second: 0,
                enabled: false
            },
            gotActivationTime: false,
            activationTimeChanged: false
        };
    },
    watch: {
        activationTime: {
            handler(value) {
                this.activationTimeChanged = true;
            },
            deep: true
        }
    },
    methods: {
        requestStatus() {
            try {
                this.ws.sendObj({ request: 'status' });
            } catch (error) {
                this.gotDateTime = false;
                this.gotSbcStatus = false;
            }
        },
        //
        updateTime(timeData) {
            if (timeData && 'value' in timeData && !('err' in timeData)) {
                this.$set(this, 'currentDateTime', timeData.value);
                this.gotDateTime = true;
            } else {
                this.gotDateTime = false;
            }
        },
        //
        onSaveActivationTimeBtnClick: function () {
            if (this.activationTime.hour >= 0 && this.activationTime.hour <= 23 &&
                this.activationTime.minute >= 0 && this.activationTime.minute <= 59 &&
                this.activationTime.second >= 0 && this.activationTime.second <= 59) {
                this.ws.sendObj({
                    request: 'saveActivationTime',
                    activationTime: this.activationTime
                });
                this.activationTimeChanged = false;
            } else {
                this.notyf.open({
                    type: 'error',
                    message: "Invalid time value(s)! Please check...",
                    duration: 2000
                });
            }
        },
        //
        onSBCTogglePowerBtnClick: function () {
            this.notyf.open({
                type: 'info',
                message: 'Requesting setPower: ' + !this.sbcPowered
            });
            this.ws.sendObj({ request: 'setPower', status: !this.sbcPowered });
        }
    },
    mounted() {
        console.log('App started');
        this.ws = new ReconnectingWebSocket('ws://' + ((deviceIpAddress) ? deviceIpAddress : location.hostname) + '/ws');
        //
        this.notyf = new Notyf({
            duration: 1000,
            position: {
                x: 'right',
                y: 'top'
            },
            types: [
                {
                    type: 'info',
                    background: '#007bff',
                    icon: false
                }
            ]
        });
        //
        // 
        this.ws.onmessage = (msg) => {
            console.log('msg.data', msg.data);
            try {
                var payload = JSON.parse(msg.data);
                console.log('parsed', payload);
                if (payload) {
                    if ('response' in payload && payload.response) {
                        //
                        if (payload.response === 'status') {
                            if ('data' in payload && payload.data) {
                                // updates system status
                                if ('sbcPowered' in payload.data) {
                                    this.gotSbcStatus = true;
                                    this.sbcPowered = payload.data.sbcPowered;
                                    this.sbcPowerStatusString = payload.data.sbcPowered ? 'ON' : 'OFF';
                                    this.sbcPowerButtonText = 'Turn ' + (payload.data.sbcPowered ? 'OFF' : 'ON');
                                    // this.$set(this, 'sbcPowered', payload.data.sbcPowered);
                                    // this.$set(this, 'sbcPowerStatusString', payload.data.sbcPowered ? 'on' : 'off');
                                }
                                // updates time
                                if ('time' in payload.data) {
                                    this.updateTime(payload.data.time);
                                }
                                // activation time
                                if (!this.gotActivationTime && 'activationTime' in payload.data) {
                                    if ('hour' in payload.data.activationTime && 'minute' in payload.data.activationTime && 'second' in payload.data.activationTime && 'enabled' in payload.data.activationTime) {
                                        this.activationTime.hour = payload.data.activationTime.hour;
                                        this.activationTime.minute = payload.data.activationTime.minute;
                                        this.activationTime.second = payload.data.activationTime.second;
                                        this.activationTime.enabled = payload.data.activationTime.enabled;
                                        this.gotActivationTime = true;
                                    }
                                }
                            }
                        }
                        //
                    }
                    //
                    if ('event' in payload && payload.event) {
                        console.log('payload', payload);
                        //
                        switch (payload.event) {
                            case 'savingActivationTime':
                                this.notyf.open({
                                    type: 'info',
                                    message: 'Saving activation time...'
                                });
                                break;
                            //
                            case 'savedActivationTime':
                                this.notyf.success("Activation time saved.");
                                break;
                            //
                            case 'timerTriggered':
                                this.notyf.success("Power ON alarm triggered!");
                                break;
                            //
                            case 'setPower':
                                if ('status' in payload) {
                                    this.notyf.success("Set SBC Power: " + (payload.status ? "ON" : "OFF"));
                                }
                                break;
                        }
                    }
                }
            } catch (error) {
                console.log('[E] error parsing message', error);
            }
        }
        //
        // this.ws.onDisconnect = () => {
        //     console.log('websocket disconnect');
        // }
        //
        this.interval = setInterval(this.requestStatus, 1000);
    }
})
