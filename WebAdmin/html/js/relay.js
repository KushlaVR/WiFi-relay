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
        Converters.ConvertAll();
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
                this.turnOff(cb.data("switch"));
            }
            else {
                this.turnOn(cb.data("switch"));
            }
        });
        $(".switch-checkbox").change(function () {
            if (this.checked) {
                this.turnOn($(this).data("switch"));
            }
            else {
                this.turnOff($(this).data("switch"));
            }
        });
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
    };
    return SetupPage;
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