using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc;
using WebAdmin.Models;
using System.Runtime.InteropServices;

namespace WebAdmin.Controllers
{
    public class ElementController : Controller
    {

        private IHostingEnvironment _env;
        public ElementController(IHostingEnvironment env)
        {
            _env = env;
        }

        public IActionResult Index()
        {
            var webRoot = _env.WebRootPath;
            var file = System.IO.Path.Combine(webRoot, "test.txt");
            System.IO.File.WriteAllText(file, "Hello World!");
            return View();
        }
        public IActionResult Get()
        {
            var webRoot = _env.WebRootPath + "\\content\\";

            if (System.IO.Directory.Exists(webRoot))
            {
                string ret = "";
                string var = null;
                String content = System.IO.File.ReadAllText(webRoot + Request.Query["name"] + ".txt");
                int i = 0;
                while (i < content.Length)
                {
                    char c = content[i];
                    if (var == null)
                    {
                        if (c == '[') { var = ""; } else { ret += c; }
                    }
                    else
                    {
                        if (c == ']')
                        {
                            ret += Request.Query[var];
                            var = null;
                        }
                        else { var += c; }
                    }
                    i++;
                };
                return Content(ret);
            }
            return NotFound();
        }

    }
}
