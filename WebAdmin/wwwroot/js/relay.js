var __extends = (this && this.__extends) || (function () {
    var extendStatics = Object.setPrototypeOf ||
        ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
        function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var WebUI = (function () {
    function WebUI() {
    }
    WebUI.prototype.init = function () {
        var setup = Model.getUrlParameter("setup");
        if (setup === undefined) {
            this.model = new HomePage();
        }
        else {
            this.model = new SetupPage();
        }
        this.model.init();
    };
    WebUI.rooturl = "api/";
    return WebUI;
}());
var Model = (function () {
    function Model() {
        this.templates = new Array();
        this.elements = new Array();
    }
    Model.getUrlParameter = function (sParam) {
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
    Model.prototype.init = function () {
        var qm = $("#questionModal");
        if (qm.length > 0)
            Relay.questionTemplate = $("#questionModal")[0].innerHTML;
    };
    Model.prototype.fillTemplate = function (t, item) {
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
    Model.prototype.loadFailed = function (data, retyHandler) {
        this.updateTime(data);
        $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
        $(".refresh").click(function () {
            if (retyHandler)
                retyHandler();
        });
    };
    Model.prototype.updateTime = function (data) {
        if (data.systime) {
            $("#systime").html(data.systime);
        }
        ;
    };
    Model.prototype.loadTemplateByName = function (name, onDone) {
        var _this = this;
        $.get(WebUI.rooturl + "template?name=" + name).done(function (data, status) { return _this.templateByNameLoaded(data, onDone); }).fail(function (data, status) { return _this.loadFailed(data, _this.loadElementsTemplate); });
    };
    Model.prototype.templateByNameLoaded = function (data, onDone) {
        this.updateTime(data);
        var item = new keyValue();
        item.key = name;
        item.value = data;
        this.templates.push(item);
        onDone();
    };
    Model.prototype.getTemplate = function (s) {
        for (var i = 0; i < this.templates.length; i++) {
            var item = this.templates[i];
            if (item.key === s) {
                return item.value;
            }
        }
        ;
        return undefined;
    };
    Model.prototype.allElementsTemplateLoaded = function (onDone) {
        for (var i = 0; i < Relay.relay.elements.length; i++) {
            var item = Relay.relay.elements[i];
            var templateName = item.template;
            var t = void 0;
            if (!(templateName === undefined)) {
                t = this.getTemplate(templateName);
                if (t === undefined) {
                    this.loadTemplateByName(templateName, onDone);
                    return false;
                }
            }
            templateName = item.editingtemplate;
            if (!(templateName === undefined)) {
                t = this.getTemplate(templateName);
                if (t === undefined) {
                    this.loadTemplateByName(templateName, onDone);
                    return false;
                }
            }
            templateName = item.visual;
            if (!(templateName === undefined)) {
                t = this.getTemplate(templateName);
                if (t === undefined) {
                    this.loadTemplateByName(templateName, onDone);
                    return false;
                }
            }
        }
        ;
        return true;
    };
    Model.prototype.loadElementsTemplate = function () {
        var _this = this;
        var allReady = this.allElementsTemplateLoaded(function () { return _this.loadElementsTemplate(); });
        if (allReady === false)
            return;
        var list = "";
        for (var i = 0; i < this.elements.length; i++) {
            var item = this.elements[i];
            if (item.visual) {
                var t = this.getTemplate(item.visual);
                if (t === undefined) {
                    list += "<li class='nav-item'>";
                    list += item.name;
                    list += "</li>";
                }
                else {
                    list += this.fillTemplate(t, item);
                }
            }
        }
        $(".process-list").html(list);
        this.allLoaded();
        Relay.ConvertAll();
    };
    Model.prototype.allLoaded = function () { };
    return Model;
}());
var HomePage = (function (_super) {
    __extends(HomePage, _super);
    function HomePage() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    HomePage.prototype.init = function () {
        _super.prototype.init.call(this);
        this.loadProcesses();
    };
    HomePage.prototype.loadProcesses = function () {
        var _this = this;
        $.get(WebUI.rooturl + "switches").done(function (data, status) { return _this.ProcessesLoaded(data, status); }).fail(function (data, status) { return _this.loadFailed(data, _this.loadProcesses); });
    };
    HomePage.prototype.ProcessesLoaded = function (data, status) {
        this.updateTime(data);
        var w = data;
        var list = "";
        for (var i = 0; i < w.items.length; i++) {
            this.elements.push(w.items[i]);
        }
        ;
        this.loadElementsTemplate();
    };
    HomePage.prototype.allLoaded = function () {
        _super.prototype.allLoaded.call(this);
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
    };
    return HomePage;
}(Model));
var SetupPage = (function (_super) {
    __extends(SetupPage, _super);
    function SetupPage() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    SetupPage.prototype.init = function () {
        _super.prototype.init.call(this);
    };
    return SetupPage;
}(Model));
var Relay = (function () {
    function Relay() {
        this.rooturl = "api/";
        this.templates = new Array();
        this.elements = new Array();
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
                Relay.relay.elements.push(item);
            }
            ;
            Relay.relay.loadProcessTemplate();
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
                Relay.relay.elements.push(item);
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
    Relay.prototype.allElementsTemplateLoaded = function (onDone) {
        for (var i = 0; i < Relay.relay.elements.length; i++) {
            var item = Relay.relay.elements[i];
            var templateName = item.template;
            var t = void 0;
            if (!(templateName === undefined)) {
                t = Relay.relay.getTemplate(templateName);
                if (t === undefined) {
                    Relay.relay.loadTemplateByName(templateName, onDone);
                    return false;
                }
            }
            templateName = item.editingtemplate;
            if (!(templateName === undefined)) {
                t = Relay.relay.getTemplate(templateName);
                if (t === undefined) {
                    Relay.relay.loadTemplateByName(templateName, onDone);
                    return false;
                }
            }
            templateName = item.visual;
            if (!(templateName === undefined)) {
                t = Relay.relay.getTemplate(templateName);
                if (t === undefined) {
                    Relay.relay.loadTemplateByName(templateName, onDone);
                    return false;
                }
            }
        }
        ;
        return true;
    };
    Relay.prototype.loadProcessTemplate = function () {
        var allReady = Relay.relay.allElementsTemplateLoaded(Relay.relay.loadProcessTemplate);
        if (allReady === false)
            return;
        var list = "";
        for (var i = 0; i < Relay.relay.elements.length; i++) {
            var item = Relay.relay.elements[i];
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
        }
    };
    Relay.prototype.loadTriggerTemplate = function () {
        var allReady = Relay.relay.allElementsTemplateLoaded(Relay.relay.loadTriggerTemplate);
        if (allReady === false)
            return;
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
            for (var i = 0; i < Relay.relay.elements.length; i++) {
                var item = Relay.relay.elements[i];
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
        var item = this.elements[i];
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