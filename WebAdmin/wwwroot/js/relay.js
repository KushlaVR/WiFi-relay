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
            this.loadTemplateByName("switch", this.loadProcesses);
        }
        else {
            $("#pageHeader").html("Налаштування");
            this.loadSetup(setup, this.getUrlParameter("index"));
        }
        var qm = $("#questionModal");
        if (qm.length > 0)
            Relay.questionTemplate = $("#questionModal")[0].innerHTML;
    };
    Relay.prototype.loadProcesses = function () {
        $.get(Relay.relay.rooturl + "switches").done(function (data, status) {
            if (data.systime) {
                $("#systime").html(data.systime);
            }
            ;
            var w = data;
            var list = "";
            for (var i = 0; i < w.items.length; i++) {
                var item = w.items[i];
                if (item.visual) {
                    var t = Relay.relay.getTemplate(item.visual);
                    if (t === undefined) {
                        list += "<li class='nav-item'>";
                        list += item.name;
                        list += "</li>";
                    }
                    else {
                        list += Relay.relay.fillTemplate(t, item);
                    }
                }
            }
            ;
            $(".process-list").html(list);
            $(".switch-checkbox[data-state='ON']").prop("checked", true);
            $("img[data-state='ON']").removeClass("light-off");
            $(".switch-img").click(function () {
                var cb = $(".switch-checkbox", $(this).closest(".card"));
                if (cb.data("state") == "ON") {
                    Relay.relay.turnOff(cb.data("switch"));
                }
                else {
                    Relay.relay.turnOn(cb.data("switch"));
                }
            });
            $(".switch-checkbox").change(function () {
                if (this.checked) {
                    Relay.relay.turnOn($(this).data("switch"));
                }
                else {
                    Relay.relay.turnOff($(this).data("switch"));
                }
            });
            Relay.ConvertAll();
        }).fail(function (data, status) {
            if (data.systime) {
                $("#systime").html(data.systime);
            }
            ;
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadProcesses();
            });
        });
    };
    Relay.prototype.loadSetup = function (setup, index) {
        $.get(Relay.relay.rooturl + "setup?type=switch&index=" + index.toString()).done(function (data, status) {
            if (data.systime) {
                $("#systime").html(data.systime);
            }
            ;
            for (var i = 0; i < data.items.length; i++) {
                var item = data.items[i];
                Relay.relay.triggers.push(item);
            }
            ;
            Relay.relay.loadTriggerTemplate();
        }).fail(function (data, status) {
            if (data.systime) {
                $("#systime").html(data.systime);
            }
            ;
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadSetup(setup, index);
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
    Relay.prototype.loadTemplateByName = function (name, onDone) {
        $.get(Relay.relay.rooturl + "template?name=" + name).done(function (data, status) {
            if (data.systime) {
                $("#systime").html(data.systime);
            }
            ;
            var item = new keyValue();
            item.key = name;
            item.value = data;
            Relay.relay.templates.push(item);
            onDone();
        }).fail(function (data, status) {
            if (data.systime) {
                $("#systime").html(data.systime);
            }
            ;
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadTriggerTemplate();
            });
        });
    };
    Relay.prototype.loadTriggerTemplate = function () {
        var allReady = true;
        for (var i = 0; i < Relay.relay.triggers.length; i++) {
            var item = Relay.relay.triggers[i];
            var t = Relay.relay.getTemplate(item.template);
            if (t === undefined) {
                allReady = false;
                Relay.relay.loadTemplateByName(item.template, Relay.relay.loadTriggerTemplate);
                return;
            }
            t = Relay.relay.getTemplate(item.editingtemplate);
            if (t === undefined) {
                allReady = false;
                Relay.relay.loadTemplateByName(item.editingtemplate, Relay.relay.loadTriggerTemplate);
                return;
            }
        }
        ;
        if (allReady === true) {
            var t = Relay.relay.getTemplate("newitem");
            if (t === undefined) {
                allReady = false;
                Relay.relay.loadTemplateByName("newitem", Relay.relay.loadTriggerTemplate);
                return;
            }
        }
        if (allReady === true) {
            var list = "";
            list += Relay.relay.getTemplate("newitem");
            for (var i = 0; i < Relay.relay.triggers.length; i++) {
                var item = Relay.relay.triggers[i];
                var t = Relay.relay.getTemplate(item.template);
                list += "<div class='holder' id='trigger_" + item.uid + "' data-index='" + i.toString() + "' data-uid='" + item.uid + "'>" + Relay.relay.fillTemplate(t, item) + "</div>";
            }
            $(".process-list").html(list);
            $(".btn-edit").click(function (e) {
                Relay.relay.edit(e);
            });
            $(".btn-add").click(function (e) {
                Relay.relay.add(e);
            });
            $('.btn-delete').click(function (e) {
                Relay.relay.delete(e);
            });
            $(".switch-checkbox[data-state='on']").prop("checked", true);
            var cp = $('.clockpicker');
            cp.clockpicker();
            Relay.ConvertAll();
        }
    };
    Relay.prototype.add = function (e) {
        console.log("add");
        var item = $(e.target).data("newitem");
        var t = this.getTemplate(item.editingtemplate);
        if (t === undefined) {
            Relay.relay.loadTemplateByName(item.editingtemplate, function () {
                Relay.relay.add(e);
            });
            return;
        }
        var iDiv = document.createElement('div');
        iDiv.id = 'trigger_0';
        iDiv.className = 'holder';
        $(".process-list")[0].appendChild(iDiv);
        var holder = $(iDiv);
        holder.data("uid", 0);
        holder.data("edit", true);
        holder.html(this.fillTemplate(t, item));
        $('.btn-edit').attr("disabled", "disabled");
        $('.btn-save').click(function (e) {
            Relay.relay.save(e);
        });
        $(".switch-checkbox[data-state='on']").prop("checked", true);
        var cp = $('.clockpicker');
        cp.clockpicker();
        Relay.ConvertAll();
    };
    Relay.prototype.edit = function (e) {
        console.log("edit");
        var holder = $(e.target).closest(".holder");
        holder.data("edit", true);
        var i = holder.data("index");
        var item = this.triggers[i];
        var t = this.getTemplate(item.editingtemplate);
        holder.html(this.fillTemplate(t, item));
        $('.btn-edit').attr("disabled", "disabled");
        $('.btn-save').click(function (e) {
            Relay.relay.save(e);
        });
        $(".switch-checkbox[data-state='on']").prop("checked", true);
        var cp = $('.clockpicker');
        cp.clockpicker();
        Relay.ConvertAll();
    };
    Relay.prototype.save = function (e) {
        console.log("save");
        var holder = $(e.target).closest(".holder");
        var form = $("form", holder);
        var fields = $("[name]", form);
        var url = Relay.relay.rooturl + "setup?switch=" + this.getUrlParameter("index") + "&uid=" + holder.data("uid");
        for (var i = 0; i < fields.length; i++) {
            var v = void 0;
            var f = $(fields[i]);
            if (f.hasClass("converter-time"))
                v = TimeConverter.ConvertBack(fields[i]);
            else if (f.hasClass("converter-week-days"))
                v = WeekDaysConverter.ConvertBack(fields[i]);
            else
                v = TextConverter.ConvertBack(fields[i]);
            url += "&" + fields[i].getAttribute("name") + "=" + encodeURIComponent(v);
        }
        $.get(url).done(function (data, status) {
            location.reload();
        }).fail(function (data, status) {
            if (data.systime) {
                $("#systime").html(data.systime);
            }
            ;
            alert(data);
        });
    };
    Relay.prototype.delete = function (e) {
        var _this = this;
        console.log("delete");
        Relay.ask("Вилучити тригер зі списку?", function () {
            var holder = $(e.target).closest(".holder");
            var form = $("form", holder);
            var url = Relay.relay.rooturl + "setup?switch=" + _this.getUrlParameter("index") + "&delete=" + holder.data("uid");
            $.get(url).done(function (data, status) {
                location.reload();
            }).fail(function (data, status) {
                if (data.systime) {
                    $("#systime").html(data.systime);
                }
                ;
                alert(data);
            });
        });
    };
    Relay.prototype.fillTemplate = function (t, item) {
        var ret = t;
        for (var key in item) {
            var s = item[key];
            var k = "@" + key + "@";
            while (ret.indexOf(k) >= 0) {
                ret = ret.replace(k, s);
            }
        }
        return ret;
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
            if (data.systime) {
                $("#systime").html(data.systime);
            }
            ;
            $("#switch_" + i.toString()).data("state", "ON");
            $("#switch_img_" + i.toString()).removeClass("light-off").addClass("light-on");
            $("#switch_" + i.toString()).prop("checked", true);
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
            if (data.systime) {
                $("#systime").html(data.systime);
            }
            ;
            $("#switch_" + i.toString()).data("state", "OFF");
            $("#switch_img_" + i.toString()).removeClass("light-on").addClass("light-off");
            $("#switch_" + i.toString()).prop("checked", false);
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
    Relay.ConvertAll = function () {
        TimeConverter.Convert();
        WeekDaysConverter.Convert();
    };
    Relay.ask = function (question, yes, no) {
        if (no === void 0) { no = null; }
        var questionModal = $("#questionModal");
        questionModal.html(Relay.questionTemplate);
        var modaTitle = $(".modal-title", questionModal);
        var modalText = $(".modal-text", questionModal);
        var btnYes = $(".btn-yes", questionModal);
        var btnNo = $(".btn-no", questionModal);
        var btnCancel = $(".btn-cancel", questionModal);
        modalText.html(question);
        questionModal.addClass("in").attr("style", "display:block;opacity:1");
        btnYes.click(function () {
            yes();
            return false;
        });
        btnNo.click(function () {
            if (no)
                no();
            questionModal.removeClass("in").removeAttr("style");
            return false;
        });
        btnCancel.click(function () {
            questionModal.removeClass("in").removeAttr("style");
            return false;
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
var TimeConverter = (function () {
    function TimeConverter() {
    }
    TimeConverter.Convert = function () {
        var inputs = $(".converter-time")
            .each(function (index, elem) {
            var el = $(elem);
            if (el.data("converted") === "1")
                return;
            var v;
            if (elem.tagName === "INPUT") {
                v = el.val();
            }
            else {
                v = elem.innerHTML;
            }
            var res = "";
            var i = parseInt(v, 10);
            res = TimeConverter.pad((Math.floor(i / 60)).toString(), 2) + ":" + TimeConverter.pad((i % 60).toString(), 2);
            el.data("converted", "1");
            if (elem.tagName === "INPUT") {
                el.attr("value", res);
            }
            else {
                elem.innerHTML = res;
            }
        });
    };
    TimeConverter.ConvertBack = function (elem) {
        var el = $(elem);
        if (el.data("converted") === "1") {
            var v = void 0;
            if (elem.tagName === "INPUT") {
                v = el.val();
            }
            else {
                v = elem.innerHTML;
            }
            var parts = v.split(":");
            var res = parseInt(parts[0]) * 60 + parseInt(parts[1]);
            return res.toString();
        }
        return el.val();
    };
    TimeConverter.pad = function (value, size) {
        var s = value;
        while (s.length < (size || 2)) {
            s = "0" + s;
        }
        return s;
    };
    return TimeConverter;
}());
var TextConverter = (function () {
    function TextConverter() {
    }
    TextConverter.ConvertBack = function (elem) {
        var el = $(elem);
        if (el.attr("type") === "checkbox") {
            return el.prop("checked");
        }
        return el.val();
    };
    return TextConverter;
}());
var WeekDaysConverter = (function () {
    function WeekDaysConverter() {
    }
    WeekDaysConverter.Convert = function () {
        var inputs = $(".converter-week-days")
            .each(function (index, elem) {
            var el = $(elem);
            if (el.data("converted") === "1")
                return;
            var v;
            if (elem.tagName === "INPUT") {
                v = el.val();
            }
            else {
                v = elem.innerHTML;
            }
            var res = "";
            var i = parseInt(v, 10);
            for (var day = 0; day < 7; day++) {
                if ((i & (1 << day)) != 0) {
                    if (res.length > 0)
                        res += ", ";
                    res += WeekDaysConverter.weekday[day];
                }
            }
            el.data("converted", "1");
            if (elem.tagName === "INPUT") {
                el.attr("value", res);
            }
            else {
                elem.innerHTML = res;
            }
        });
    };
    WeekDaysConverter.ConvertBack = function (elem) {
        var el = $(elem);
        if (el.data("converted") === "1") {
            var v = void 0;
            if (elem.tagName === "INPUT") {
                v = el.val();
            }
            else {
                v = elem.innerHTML;
            }
            var parts = v.split(",");
            var res = 0;
            for (var i = 0; i < parts.length; i++) {
                var n = WeekDaysConverter.weekday.indexOf(parts[i].trim());
                if (n >= 0)
                    res += (1 << n);
            }
            return res.toString();
        }
        return el.val();
    };
    WeekDaysConverter.weekday = ["Нд", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб"];
    return WeekDaysConverter;
}());
//# sourceMappingURL=relay.js.map