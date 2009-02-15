/**
* Appcelerator Titanium - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace Titanium
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            string [] args = Environment.GetCommandLineArgs();
            if (args.Length < 8)
            {
                MessageBox.Show("Invalid arguments passed to Installer","Error",MessageBoxButtons.OK,MessageBoxIcon.Error);
                return;
            }

            string appname = args[1];
            string title = args[2];
            string message = args[3];
            string appTitle = appname + " Installer";
            string tempdir = args[4];
            string installdir = args[5];
            string unzipper = args[6];

            // do the confirmation for the user
            DialogResult result = MessageBox.Show(message, title, MessageBoxButtons.OKCancel, MessageBoxIcon.Information, MessageBoxDefaultButton.Button1, MessageBoxOptions.ServiceNotification);
            if (result != DialogResult.OK)
            {
                MessageBox.Show("Installation Aborted. To install later, re-run the application again.", title, MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            int count = args.Length - 7;
            string[] urls = new string[count];
            int x = 0;

            for (int c = 7; c < args.Length; c++)
            {
                urls[x++] = args[c];
            }
            Application.Run(new form(tempdir,installdir,appTitle,urls,unzipper));
        }
    }
}
