using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.IO;
using System.Diagnostics;

namespace Titanium
{

    public partial class form : Form
    {
        public form(string tempdir, string installdir, string title, string[] urls, string unzipper)
        {
            InitializeComponent();

            this.label.Text = "Preparing to download ...";
            this.progress.Style = ProgressBarStyle.Continuous;
            this.Text = title;
            this.urls = urls;
            this.tempdir = tempdir;
            this.installdir = installdir;
            this.unzipper = unzipper;
            this.textDelegate = new UpdateTextDelegate(this.UpdateText);
            this.progressDelegate = new UpdateProgressDelegate(this.UpdateProgress);
        }

        private void button_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void backgroundWorker_DoWork_1(object sender, DoWorkEventArgs e)
        {
            try
            {
                IFormatProvider provider = new Titanium.FileSizeFormatProvider();

                this.Invoke(this.textDelegate, new object[]{
                    "Preparing to download " + this.urls.Length + " file" + (this.urls.Length > 1 ? "s" : "")    
                });

                string [] files = new string[this.urls.Length];

                for (int c = 0; c < this.urls.Length; c++)
                {
                    Uri uri = new Uri(this.urls[c]);
                    string[] segments = uri.Segments;
                    string filename = segments[segments.Length - 1];
                    string path = tempdir + "//" + filename;
                    files[c] = filename;

                    HttpWebRequest request = (HttpWebRequest)WebRequest.Create(uri);
                    request.UserAgent = "Mozilla/5.0 (compatible; Titanium_Downloader/0.2; Win32)";
                    request.Method = "GET";

                    string currentMessage = "Downloading " + (c + 1) + " of " + this.urls.Length + " ... ";

                    this.Invoke(this.textDelegate, new object[]{
                        currentMessage
                    });

                    HttpWebResponse response = (HttpWebResponse)request.GetResponse();

                    string totalDisplay = String.Format(provider, "{0:fs}", response.ContentLength);

                    this.Invoke(this.progressDelegate, new object[]{
                        0,
                        (int)response.ContentLength
                    });

                    Stream inStream = response.GetResponseStream();
                    if (File.Exists(path))
                    {
                        File.Delete(path);
                    }
                    FileStream outStream = File.Create(path);
                    byte[] buffer = new byte[8096];
                    int total = 0;

                    while (true)
                    {
                        int count = inStream.Read(buffer, 0, 8096);
                        if (count == 0) break;
                        total += count;
                        outStream.Write(buffer, 0, count);
                        this.Invoke(this.progressDelegate, new object[]{
                            total,
                            (int)response.ContentLength
                        });
                        this.Invoke(this.textDelegate, new object[]{
                            currentMessage +
                            String.Format(provider, "{0:fs}", total) + " of " +
                            totalDisplay
                        });
                    }

                    response.Close();
                    inStream.Close();
                    outStream.Close();
                }

                for (int c = 0; c < files.Length; c++)
                {
                    string currentMessage = "Installing " + (c + 1) + " of " + files.Length + " ... ";
                    this.Invoke(this.textDelegate, new object[]{
                        currentMessage
                    });

                    string name = files[c];
                    string [] tokens = name.Split('-');
                    string type = tokens[0];
                    string subtype = tokens[1];
                    string version = tokens[2].Substring(0, tokens[2].LastIndexOf("."));
    
                    string destdir;

                    if (type == "runtime")
                    {
                        destdir = installdir + "\\runtime\\" + subtype + "\\" + version;
                    }
                    else if (type == "module")
                    {
                        destdir = installdir + "\\modules\\" + subtype + "\\" + version;
                    }
                    else
                    {
                        continue;
                    }

                    string from = tempdir+"\\"+name;
                    string to = destdir+"\\"+name;

                    // in win32, we just invoke back the same process and let him unzip
                    Process p = new Process();
                    p.StartInfo.UseShellExecute = false;
                    p.StartInfo.WorkingDirectory = Application.StartupPath;
                    p.StartInfo.Arguments = "--tiunzip \"" + from + "\" \"" + to + "\"";
                    p.StartInfo.CreateNoWindow = true;
                    p.StartInfo.FileName = this.unzipper;
                    p.Start();
                    p.WaitForExit();

                    // delete the temp file and cleanup
                    File.Delete(this.tempdir + "\\" + files[c]);

                    // update the progress indicator
                    this.Invoke(this.progressDelegate, new object[]{
                            c+1,
                            files.Length
                        });
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message+"\n\n"+ex.StackTrace);
            }
            Application.Exit();
        }

        private void form_Load(object sender, EventArgs e)
        {
            backgroundWorker.RunWorkerAsync();
        }

    }
}