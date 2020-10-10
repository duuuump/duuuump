const PhoneConnector = (function () {
    const _ws = Symbol('_ws');
    const _state = Symbol('_state');
    const _cmd_map = Symbol('_cmd_map');
    const _msg_dom = Symbol('_msg_dom');
    const _connect = Symbol('_msg_dom');
    let _ws_ping_timer = null;
    let _ws_reconnect_timer = null;
    let _off_tip_f = true;
    let _count_retry = 0;

    let _comming_yes = Symbol('_comming_yes');
    let _comming_no = Symbol('_comming_no');
    let _current_cid = null;

    const log = console.log;

    class Connector {
        constructor() {
            this[_state] = false;
            this[_cmd_map] = {};
            this[_connect] = () => {
                this[_ws] = new WebSocket('ws://127.0.0.1:4399');
                this[_ws].onopen = () => {
                    this.message("话机服务连接");
                    _ws_ping_timer = setInterval(() => {
                        this[_ws].send("ping");
                    }, 1000 * 60);
                    if (_ws_reconnect_timer) {
                        clearTimeout(_ws_reconnect_timer);
                        _ws_reconnect_timer = null;
                    }
                    let href = window.location.href;
                    href = href.substring(0, href.lastIndexOf("/") + 1);
                    href += '--上传地址--';
                    this[_ws].send(JSON.stringify({cmd: 'set_upload', data: href}));
                    _count_retry = 0;
                    _off_tip_f = true;
                    this[_state] = true;
                };
                this[_ws].onclose = () => {
                    if (_off_tip_f) {
                        this.message("话机服务未连接");
                        _off_tip_f = false;
                    }
                    if (_ws_ping_timer) {
                        clearInterval(_ws_ping_timer);
                        _ws_ping_timer = null;
                    }
                    if (_count_retry < 10) {
                        _ws_reconnect_timer = setTimeout(() => {
                            this[_connect]();
                            clearTimeout(_ws_reconnect_timer);
                            _ws_reconnect_timer = null;
                        }, 6000);
                        _count_retry ++;
                        log('--[Socket]--第' + _count_retry + '次尝试重连');
                    }
                    this[_state] = false;
                };
                this[_ws].onmessage = (evt) => {
                    try {
                        let msg = JSON.parse(evt.data);
                        log(msg);
                        if (msg.event) {
                            switch (msg.event) {
                                case 'call': {
                                    const fun = this[_cmd_map]['call']['ifSuccess'];
                                    const phone = this[_cmd_map]['call']['phone'];
                                    clearTimeout(this[_cmd_map]['call']['timeout']);
                                    this[_cmd_map]['call'] = null;
                                    if (msg.res.indexOf('成功') > -1) {
                                        _current_cid = msg.cid;
                                        this.message('拨号: ' + phone);
                                        if (typeof fun === 'function') {
                                            fun();
                                        }
                                    } else {
                                        this.message(msg.res);
                                    }
                                    break;
                                }
                                case 'hang': {
                                    this.message(msg.res);
                                    const fun = this[_cmd_map]['hang']['ifSuccess'];
                                    clearTimeout(this[_cmd_map]['hang']['timeout']);
                                    this[_cmd_map]['hang'] = null;
                                    if (typeof fun === 'function') {
                                        fun();
                                    }
                                    break;
                                }
                                case 'hook': {
                                    this.message(msg.res);
                                    const fun = this[_cmd_map]['hook']['ifSuccess'];
                                    clearTimeout(this[_cmd_map]['hook']['timeout']);
                                    this[_cmd_map]['hook'] = null;
                                    if (msg.res.indexOf('成功') > -1 && typeof fun === 'function') {
                                        _current_cid = msg.cid;
                                        fun();
                                    }
                                    break;
                                }
                                case 'comming': {
                                    const phone = msg.res;
                                    layer.confirm('客户来电:' + phone, {id: 'onlyOne'}, index => {
                                        if (typeof this[_comming_yes] === 'function') {
                                            this[_comming_yes](phone);
                                        } else {
                                            layer.msg('客户来电:' + phone);
                                        }
                                        layer.close(index);
                                    }, () => {
                                        if (typeof this[_comming_no] === 'function') {
                                            this[_comming_no](phone);
                                        }
                                    });
                                }
                            }
                        }
                    } catch (e) {
                        log(e);
                    }
                };
            };
            this[_connect]();
        }

        async State() {
            if ([0, 2, 3].includes(this[_ws].readyState)) {
                await new Promise(resolve => {
                    if (this[_ws].readyState !== this[_ws].CONNECTING) {
                        log('--[Socket]--断线重连');
                        this[_connect]();
                    }
                    let tt = setTimeout(() => {
                        resolve();
                        clearTimeout(tt);
                    }, 1500);
                });
            }
            return this[_state];
        }

        MsgDom(dom) {
            if (dom) {
                this[_msg_dom] = dom;
            }
            return this[_msg_dom];
        }

        message(m) {
            // 在顶层窗口, 先检查dom归属的document是否存在
            if (this[_msg_dom] && this[_msg_dom].ownerDocument.visibilityState === 'visible') {
                const p = document.createElement('p');
                p.className = 'mess-p';
                p.innerText = "- "+new Date().toLocaleTimeString() + " " + m+" -";
                this[_msg_dom].insertBefore(p, this[_msg_dom].childNodes[0]);
            } else {
                layer.msg(m);
            }
        }

        Call(phone, token, call) {
            this.State().then(state => {
                if (!state) {
                    return;
                }
                if (this[_cmd_map]['call']) {
                    this.message("不要重复拨号!!!");
                } else {
                    try {
                        this[_ws].send(JSON.stringify({cmd: 'call', phone: phone, token: token}));
                        this[_cmd_map]['call'] = {};
                        this[_cmd_map]['call']['ifSuccess'] = call;
                        this[_cmd_map]['call']['phone'] = phone;
                        this[_cmd_map]['call']['timeout'] = setTimeout(() => {
                            if (this[_cmd_map]['call']) {
                                clearTimeout(this[_cmd_map]['call']['timeout']);
                                this.message("拨号超时");
                                this[_cmd_map]['call'] = null;
                            }
                        }, 5000);
                    } catch (e) {
                        log(e);
                        this[_cmd_map]['call'] = null;
                    }
                }
            });
        }

        Hang(call) {
            this.State().then(state => {
                if (!state) {
                    return;
                }
                if (this[_cmd_map]['hang']) {
                    this.message("重复操作");
                } else {
                    try {
                        this[_ws].send(JSON.stringify({cmd: 'hang'}));
                        this[_cmd_map]['hang'] = {};
                        this[_cmd_map]['hang']['ifSuccess'] = call;
                        this[_cmd_map]['hang']['timeout'] = setTimeout(() => {
                            if (this[_cmd_map]['hang']) {
                                this.message("挂机超时");
                                clearTimeout(this[_cmd_map]['hang']['timeout']);
                                this[_cmd_map]['hang'] = null;
                            }
                        }, 5000);
                    } catch (e) {
                        log(e);
                        this[_cmd_map]['hang'] = null;
                    }
                }
            });
        }
        Hook(token, call) {
            this.State().then(state => {
                if (!state) {
                    return;
                }
                if (this[_cmd_map]['hook']) {
                    this.message("重复操作");
                } else {
                    try {
                        this[_ws].send(JSON.stringify({cmd: 'hook', token: token}));
                        this[_cmd_map]['hook'] = {};
                        this[_cmd_map]['hook']['ifSuccess'] = call;
                        this[_cmd_map]['hook']['timeout'] = setTimeout(() => {
                            if (this[_cmd_map]['hook']) {
                                this.message("摘机超时");
                                clearTimeout(this[_cmd_map]['hook']['timeout']);
                                this[_cmd_map]['hook'] = null;
                            }
                        }, 5000);
                    } catch (e) {
                        log(e);
                        this[_cmd_map]['hook'] = null;
                    }
                }
            });
        }

        OnComming(yes, no) {
            this[_comming_yes] = yes;
            this[_comming_no] = no;
        };

        RefreshToken(token) {
            this.State().then(state => {
                if (!state) {
                    return;
                }
                try {
                    if (_current_cid) {
                        this[_ws].send(JSON.stringify({cmd: 'refresh_token', cid: _current_cid, token: token}));
                    }
                } catch (e) {
                    log(e)
                }
            });
        }

        static Instance() {
            if (!Connector.instance) {
                Connector.instance = new Connector();
            }
            return Connector.instance;
        }
    }
    return Connector;
})();
