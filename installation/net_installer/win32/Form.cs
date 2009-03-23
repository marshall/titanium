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
        //static string DISTRIBUTION_UUID = "7F7FA377-E695-4280-9F1F-96126F3D2C2A";
        static string RUNTIME_UUID = "A2AC5CB5-8C52-456C-9525-601A5B0725DA";
        static string MODULE_UUID = "1ACE5D3A-2B52-43FB-A136-007BD166CFD0";

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

                for (int c = 0; c < this.urls.Length; c++)
                {
                    Uri uri = new Uri(this.urls[c]);
                    string filename = this.getFilename(uri);

                    if (filename == null)
                    {
                        System.Console.WriteLine("Unable to deteremine filename for " + this.urls[c]);
                        continue;
                    }

                    string path = tempdir + "\\" + filename;

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

                for (int c = 0; c < this.urls.Length; c++)
                {
                    string currentMessage = "Installing " + (c + 1) + " of " + this.urls.Length + " ... ";
                    this.Invoke(this.textDelegate, new object[]{
                        currentMessage
                    });

                    Uri uri = new Uri(this.urls[c]);

                    string filename = this.getFilename(uri);

                    if (filename == null)
                    {
                        continue;
                    }

                    string name = this.getURIParam(uri, "name");
                    string subtype = "win32";
                    string version = this.getURIParam(uri, "version");
    
                    string uuid = this.getURIParam(uri, "uuid");

                    string destdir;

                    if (RUNTIME_UUID == uuid)
                    {
                        destdir = installdir + "\\runtime\\" + subtype + "\\" + version;
                    }
                    else if (MODULE_UUID == uuid)
                    {
                        destdir = installdir + "\\modules\\" + subtype + "\\" + name + "\\" + version;
                    }
                    else
                    {
                        continue;
                    }

                    string from = tempdir + "\\" + filename;
                    string to = destdir;

                    Directory.CreateDirectory(to);

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
                    File.Delete(from);

                    // update the progress indicator
                    this.Invoke(this.progressDelegate, new object[]{
                            c+1,
                            this.urls.Length
                        });
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message+"\n\n"+ex.StackTrace);
            }
            Application.Exit();
        }

        private string getFilename(Uri uri)
        {
            string name = this.getURIParam(uri, "name");
            string version = this.getURIParam(uri, "version");

            if (name == null || version == null)
            {
                return null;
            }

            return name + "-" + version + ".zip";
        }

        private string getURIParam(Uri uri, string paramName)
        {
            string query = uri.Query;
            if (query.Length > 0 && query[0] == '?')
            {
                query = query.Substring(1);
            }

            string[] pairs = query.Split('&');

            for (int i = 0; i < pairs.Length; i++)
            {
                string pair = pairs[i];

                string[] tokens = pair.Split('=');

                if (paramName == tokens[0])
                {
                    // TODO -  ensure the value is decoded
                    return tokens[1];
                }
            }

            return null;
        }

        private void form_Load(object sender, EventArgs e)
        {
            backgroundWorker.RunWorkerAsync();
        }

    }
}