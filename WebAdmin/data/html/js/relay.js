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
    WebUI.init = function () {
        var setup = Model.getUrlParameter("setup");
        if (setup === undefined) {
            WebUI.model = new HomePage();
        }
        else {
            WebUI.model = new SetupPage();
        }
        WebUI.model.init();
    };
    WebUI.wifi = function () {
        WebUI.model = new WifiPage();
        WebUI.model.init();
    };
    WebUI.ask = function (question, yes, no) {
        if (no === void 0) { no = null; }
        var questionModal = $("#questionModal");
        questionModal.html(WebUI.questionTemplate);
        var modaTitle = $(".modal-title", questionModal);
        var modalText = $(".modal-text", questionModal);
        var btnYes = $(".btn-yes", questionModal);
        var btnNo = $(".btn-no", questionModal);
        var btnCancel = $(".btn-cancel", questionModal);
        modalText.html(question);
        questionModal.addClass("in").attr("style", "display:block;opacity:1");
        btnYes.click(function () {
            yes();
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
            WebUI.questionTemplate = $("#questionModal")[0].innerHTML;
        Converters.ClockPicker();
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
        var time_str = "";
        if (data.systime) {
            time_str += data.systime;
        }
        ;
        if (data.uptime) {
            var ms = data.uptime;
            var uptime = "";
            var second = Math.floor(ms / 1000) % 60;
            var minute = Math.floor(ms / 1000 / 60) % 60;
            var hour = Math.floor(ms / 1000 / 60 / 60) % 24;
            var days = Math.floor(ms / 1000 / 60 / 60 / 24);
            if (days > 0)
                uptime += days.toString() + " днів";
            if (days > 0 || hour > 0)
                uptime += " " + hour.toString() + ":";
            uptime += minute.toString();
            uptime += ":" + second.toString();
            time_str += " (час роблти - " + uptime + ")";
        }
        ;
        if (data.systime || data.uptime)
            $("#systime").html(time_str);
    };
    Model.prototype.loadTemplateByName = function (name, onDone) {
        var _this = this;
        var _name = name;
        $.get(WebUI.rooturl + "template?name=" + name).done(function (data, status) { return _this.templateByNameLoaded(_name, data, onDone); }).fail(function (data, status) { return _this.loadFailed(data, _this.loadElementsTemplate); });
    };
    Model.prototype.templateByNameLoaded = function (name, data, onDone) {
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
        for (var i = 0; i < this.elements.length; i++) {
            var item = this.elements[i];
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
        this.allLoaded();
        Converters.ConvertAll();
        $(".switch-checkbox[data-state='ON']").prop("checked", true);
    };
    Model.prototype.allLoaded = function () {
    };
    Model.prototype.getUrlParameter = function (sParam) {
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
        var _this = this;
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
        _super.prototype.allLoaded.call(this);
        $("img[data-state='ON']").removeClass("light-off");
        $(".switch-img").click(function (e) { return _this.switch_img_click(e); });
        $(".switch-checkbox").change(function (e) { return _this.switch_checkbox_change(e); });
    };
    HomePage.prototype.switch_img_click = function (event) {
        var cb = $(".switch-checkbox", $(event.target).closest(".card"));
        if (cb.data("state") == "ON") {
            this.turnOff(cb.data("switch"));
        }
        else {
            this.turnOn(cb.data("switch"));
        }
    };
    HomePage.prototype.switch_checkbox_change = function (event) {
        if (event.target.checked) {
            this.turnOn($(event.target).data("switch"));
        }
        else {
            this.turnOff($(event.target).data("switch"));
        }
    };
    HomePage.prototype.turnOn = function (i) {
        if ($("#switch_" + i.toString()).data("state") == "ON")
            return;
        console.log("turn on");
        $.post(WebUI.rooturl + "switches" + "?index=" + i.toString() + "&state=on").done(function (data, status) {
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
    HomePage.prototype.turnOff = function (i) {
        if ($("#switch_" + i.toString()).data("state") == "OFF")
            return;
        console.log("turn off");
        $.post(WebUI.rooturl + "switches" + "?index=" + i.toString() + "&state=off").done(function (data, status) {
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
    return HomePage;
}(Model));
var SetupPage = (function (_super) {
    __extends(SetupPage, _super);
    function SetupPage() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    SetupPage.prototype.init = function () {
        _super.prototype.init.call(this);
        this.loadTriggers();
    };
    SetupPage.prototype.loadTriggers = function () {
        var _this = this;
        var index = this.getUrlParameter("index");
        $.get(WebUI.rooturl + "setup?type=switch&index=" + index.toString()).done(function (data, status) { return _this.TriggersLoaded(data, status); }).fail(function (data, status) { return _this.loadFailed(data, _this.loadTriggers); });
    };
    SetupPage.prototype.TriggersLoaded = function (data, status) {
        this.updateTime(data);
        var w = data;
        var list = "";
        for (var i = 0; i < w.items.length; i++) {
            this.elements.push(w.items[i]);
        }
        ;
        this.loadElementsTemplate();
    };
    SetupPage.prototype.allLoaded = function () {
        var _this = this;
        var allReady = true;
        var t = this.getTemplate("newitem");
        if (t === undefined) {
            allReady = false;
            this.loadTemplateByName("newitem", function () { return _this.allLoaded(); });
            return;
        }
        if (allReady === true) {
            var list = "";
            list += this.getTemplate("newitem");
            for (var i = 0; i < this.elements.length; i++) {
                var item = this.elements[i];
                var t_1 = this.getTemplate(item.template);
                list += "<div class='holder' id='trigger_" + item.uid + "' data-index='" + i.toString() + "' data-uid='" + item.uid + "'>" + this.fillTemplate(t_1, item) + "</div>";
            }
            $(".process-list").html(list);
            $(".btn-edit").click(function (e) { return _this.edit(e); });
            $(".btn-add").click(function (e) { return _this.add(e); });
            $('.btn-delete').click(function (e) { return _this.delete(e); });
            $(".switch-checkbox[data-state='on']").prop("checked", true);
            Converters.ClockPicker();
        }
        $(".switch-checkbox[data-state='ON']").prop("checked", true);
        $("img[data-state='ON']").removeClass("light-off");
        Converters.ConvertAll();
    };
    SetupPage.prototype.add = function (e) {
        var _this = this;
        console.log("add");
        var item = $(e.target).data("newitem");
        var t = this.getTemplate(item.editingtemplate);
        if (t === undefined) {
            var _e_1 = e;
            this.loadTemplateByName(item.editingtemplate, function () { return _this.add(_e_1); });
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
        $('.btn-save').click(function (e) { return _this.save(e); });
        $(".switch-checkbox[data-state='on']").prop("checked", true);
        Converters.ConvertAll();
        Converters.ClockPicker();
    };
    SetupPage.prototype.edit = function (e) {
        var _this = this;
        console.log("edit");
        var holder = $(e.target).closest(".holder");
        holder.data("edit", true);
        var i = holder.data("index");
        var item = this.elements[i];
        var t = this.getTemplate(item.editingtemplate);
        holder.html(this.fillTemplate(t, item));
        $('.btn-edit').attr("disabled", "disabled");
        $('.btn-save').click(function (e) { return _this.save(e); });
        $(".switch-checkbox[data-state='on']").prop("checked", true);
        Converters.ConvertAll();
        Converters.ClockPicker();
    };
    SetupPage.prototype.save = function (e) {
        console.log("save");
        var holder = $(e.target).closest(".holder");
        var form = $("form", holder);
        var fields = $("[name]", form);
        var url = WebUI.rooturl + "setup?switch=" + this.getUrlParameter("index") + "&uid=" + holder.data("uid");
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
    SetupPage.prototype.delete = function (e) {
        var _this = this;
        console.log("delete");
        var _e = e;
        WebUI.ask("Вилучити тригер зі списку?", function () { return _this.onDeleConfirmed(_e); });
    };
    SetupPage.prototype.onDeleConfirmed = function (e) {
        var holder = $(e.target).closest(".holder");
        var form = $("form", holder);
        var url = WebUI.rooturl + "setup?switch=" + this.getUrlParameter("index") + "&delete=" + holder.data("uid");
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
    return SetupPage;
}(Model));
var WifiPage = (function (_super) {
    __extends(WifiPage, _super);
    function WifiPage() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    WifiPage.prototype.init = function () {
        _super.prototype.init.call(this);
        this.loadStationList();
    };
    WifiPage.prototype.loadStationList = function () {
        var _this = this;
        $.get(WebUI.rooturl + "wifi").done(function (data, status) { return _this.StationListLoaded(data, status); }).fail(function (data, status) { return _this.loadFailed(data, _this.loadStationList); });
    };
    WifiPage.prototype.StationListLoaded = function (data, status) {
        this.updateTime(data);
        var w = data;
        $(".localip").text(w.localip);
        $(".mac").text(w.mac);
        $(".dnsip").text(w.dnsip);
        $(".getway").text(w.getway);
        var list = "";
        for (var i = 0; i < w.ssid.length; i++) {
            var item = w.ssid[i];
            item.template = "wifi";
            if (item.encryption == "7") {
                item.encryption_name = "-";
            }
            else {
                item.encryption_name = "***";
            }
            this.elements.push(item);
        }
        ;
        this.loadElementsTemplate();
    };
    WifiPage.prototype.allLoaded = function () {
        var _this = this;
        var list = "";
        for (var i = 0; i < this.elements.length; i++) {
            var item = this.elements[i];
            var t = this.getTemplate(item.template);
            if (t === undefined) {
                list += "<li class='nav-item'>";
                list += item.name;
                list += "</li>";
            }
            else {
                list += this.fillTemplate(t, item);
            }
        }
        ;
        $(".wifi-list").html(list);
        $(".wifi-item").on("click", function (e) { return _this.wifi_item_click(e); });
    };
    WifiPage.prototype.wifi_item_click = function (event) {
        $('#ssid').val($(".name", event.currentTarget).text());
        $('#password').focus();
    };
    return WifiPage;
}(Model));
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
var Converters = (function () {
    function Converters() {
    }
    Converters.ConvertAll = function () {
        TimeConverter.Convert();
        TextConverter.Convert();
        WeekDaysConverter.Convert();
    };
    Converters.ClockPicker = function () {
        var cp = $(".clockpicker");
        cp.clockpicker({
            placement: 'top',
            autoclose: true
        });
    };
    return Converters;
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
    TextConverter.Convert = function () { };
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