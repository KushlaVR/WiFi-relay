var Relay = (function () {
    function Relay() {
        this.rooturl = "api/";
        this.templates = new Array();
        this.triggers = new Array();
    }
    Relay.prototype.init = function () {
        var setup = this.getUrlParameter("setup");
        if (setup === undefined) {
            $("#pageHeader").html("Стан виходів");
            this.loadProcesses();
        }
        else {
            $("#pageHeader").html("Налаштування");
            this.loadSetup(setup, this.getUrlParameter("index"));
        }
    };
    Relay.prototype.loadProcesses = function () {
        $.get(Relay.relay.rooturl + "switches").done(function (data, status) {
            var w = data;
            var list = "";
            for (var i = 0; i < w.items.length; i++) {
                var item = w.items[i];
                if (item.visual) {
                    if (item.visual == "switch") {
                        list += "<div class='card m-3' style='width: 18rem; display:inline-block;'>";
                        list += "  <img class='card-img-top light-off' src='/content/idea.svg' alt='Вимикач' id='switch_img_" + item.index + "' data-state='" + item.state + "'>";
                        list += "  <div class='card-body'>";
                        list += "    <h5 class='card-title'>Вихід №" + item.index + "</h5>";
                        list += "<label class=" + item.type + ">";
                        list += "<input type='checkbox' class='switch-checkbox' id='switch_" + item.index + "'data-switch='" + item.index + "' data-state='" + item.state + "'/>";
                        list += "<span class='slider'></span>";
                        list += "</label>";
                        list += "  </div>";
                        list += "  <ul class='list-group list-group-flush'>";
                        list += "    <li class='list-group-item'><a href='/index.html?setup=switch&index=" + item.index + "' class='card-link'>Налаштувати</a></li>";
                        list += "  </ul>";
                        list += "</div>";
                    }
                    else {
                        list += "<li class='nav-item'>";
                        list += item.name;
                        list += "</li>";
                    }
                }
            }
            ;
            $(".process-list").html(list);
            $(".switch-checkbox[data-state='ON']").prop("checked", true);
            $("img[data-state='ON']").removeClass("light-off");
            $(".switch-checkbox").change(function () {
                if (this.checked) {
                    Relay.relay.turnOn($(this).data("switch"));
                }
                else {
                    Relay.relay.turnOff($(this).data("switch"));
                }
            });
        }).fail(function (data, status) {
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadProcesses();
            });
        });
    };
    Relay.prototype.getTemplate = function (s) {
        for (var i = 0; i < this.templates.length; i++) {
            var item = this.templates[i];
            if (item.key === s) {
                return item.value;
            }
        }
        ;
        return undefined;
    };
    Relay.prototype.loadTemplateByName = function (name) {
        $.get(Relay.relay.rooturl + "template?name=" + name).done(function (data, status) {
            var item = new keyValue();
            item.key = data.name;
            item.value = data.html;
            Relay.relay.templates.push(item);
            Relay.relay.loadTemplate();
        }).fail(function (data, status) {
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadTemplate();
            });
        });
    };
    Relay.prototype.loadTemplate = function () {
        var allReady = true;
        for (var i = 0; i < this.triggers.length; i++) {
            var item = this.triggers[i];
            var t = this.getTemplate(item.template);
            if (t === undefined) {
                allReady = false;
                this.loadTemplateByName(item.template);
                return;
            }
            t = this.getTemplate(item.editingtemplate);
            if (t === undefined) {
                allReady = false;
                this.loadTemplateByName(item.editingtemplate);
                return;
            }
        }
        ;
        if (allReady === true) {
            var list = "";
            for (var i = 0; i < this.triggers.length; i++) {
                var item = this.triggers[i];
                var t = this.getTemplate(item.template);
                list += "<div style='display:inline-block;' class='holder' id='trigger_" + item.uid + "' data-index='" + i.toString() + "'>" + this.fillTemplate(t, item) + "</div>";
            }
            $(".process-list").html(list);
            $(".btn-edit").click(function (e) {
                Relay.relay.edit(e);
            });
        }
    };
    Relay.prototype.fillTemplate = function (t, item) {
        return t.replace("#name#", item.name)
            .replace("#action#", item.action)
            .replace("#type#", item.type)
            .replace("#desc#", item.desc);
    };
    Relay.prototype.edit = function (e) {
        console.log("edit");
        var holder = $(e.target).closest(".holder");
        var i = holder.data("index");
        var item = this.triggers[i];
        var t = this.getTemplate(item.editingtemplate);
        holder.html(this.fillTemplate(t, item));
    };
    Relay.prototype.loadSetup = function (setup, index) {
        $.get(Relay.relay.rooturl + "setup?type=switch&index=" + index.toString()).done(function (data, status) {
            for (var i = 0; i < data.items.length; i++) {
                var item = data.items[i];
                Relay.relay.triggers.push(item);
            }
            ;
            Relay.relay.loadTemplate();
        }).fail(function (data, status) {
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadSetup(setup, index);
            });
        });
    };
    Relay.prototype.getUrlParameter = function (sParam) {
        var sPageURL = window.location.search.substring(1);
        var sURLVariables = sPageURL.split('&');
        var sParameterName;
        var i;
        for (i = 0; i < sURLVariables.length; i++) {
            sParameterName = sURLVariables[i].split('=');
            if (sParameterName[0] === sParam) {
                return (sParameterName[1] === undefined) ? true : decodeURIComponent(sParameterName[1]);
            }
        }
        return undefined;
    };
    Relay.prototype.turnOn = function (i) {
        if ($("#switch_" + i.toString()).data("state") == "ON")
            return;
        console.log("turn on");
        $.post(Relay.relay.rooturl + "switches" + "?index=" + i.toString() + "&state=on").done(function (data, status) {
            $("#switch_" + i.toString()).data("state", "ON");
            $("#switch_img_" + i.toString()).removeClass("light-off").addClass("light-on");
        }).fail(function (data, status) {
            $("#switch_" + i.toString()).prop("checked", false);
        });
        return true;
    };
    Relay.prototype.turnOff = function (i) {
        if ($("#switch_" + i.toString()).data("state") == "OFF")
            return;
        console.log("turn off");
        $.post(Relay.relay.rooturl + "switches" + "?index=" + i.toString() + "&state=off").done(function (data, status) {
            $("#switch_" + i.toString()).data("state", "OFF");
            $("#switch_img_" + i.toString()).removeClass("light-on").addClass("light-off");
        }).fail(function (data, status) {
            $("#switch_" + i.toString()).prop("checked", true);
        });
        return true;
    };
    Relay.prototype.wifi = function () {
        $.get(Relay.relay.rooturl + "wifi").done(function (data, status) {
            var w = data;
            var list = "";
            for (var i = 0; i < w.ssid.length; i++) {
                list += "<a class='wifi-item list-group-item' href='#'>";
                list += "<div class='name'>";
                list += w.ssid[i].name;
                list += "</div>";
                list += "<div class='encryption'>";
                if (w.ssid[i].encryption == "7")
                    list += "-";
                else
                    list += "***";
                list += "</div>";
                list += "<div class='rssi'>";
                list += w.ssid[i].rssi;
                list += "</div>";
                list += "</a>";
            }
            ;
            $(".wifi-list").html(list);
            $(".wifi-item").on("click", function () {
                $('#ssid').val($(".name", this).text());
                $('#password').focus();
            });
        }).fail(function (data, status) {
            $(".wifi-list").html("Не вдалось завантажити список мереж.<br />Поновіть сторінку, щоб повторити спробу.");
        });
    };
    Relay.relay = new Relay();
    return Relay;
}());
var WIFI_list = (function () {
    function WIFI_list() {
    }
    return WIFI_list;
}());
var Items_list = (function () {
    function Items_list() {
    }
    return Items_list;
}());
var keyValue = (function () {
    function keyValue() {
    }
    return keyValue;
}());
var Trigger = (function () {
    function Trigger() {
    }
    return Trigger;
}());
//# sourceMappingURL=relay.js.map