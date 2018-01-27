using System.IO;
using System.Diagnostics;
using System.Windows;
using System;
using Microsoft.Win32;

namespace stide_gui
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private String _strict_c = "-s";
        private String _verbose_c = "-v";
        private String _p = " ";

        private String _strict_e = "-s";
        private String _verbose_e = "-v";

        public MainWindow()
        {
            InitializeComponent();
        }

        // <--- Extract
        private void img_e_click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
                img_e.Text = openFileDialog.FileName;
        }

        private void db_e_click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
                db_e.Text = openFileDialog.FileName;
        }

        private void verbose_e_set(object sender, RoutedEventArgs e)
        {
            String selectedItem = verbose_e.SelectedItem.ToString();
            if (selectedItem.Contains("silent"))
                _verbose_e = " ";
            else if (selectedItem.Contains("verbose"))
                _verbose_e = "-v";
            else
                _verbose_e = "-d";
        }

        private void strict_e_set(object sender, RoutedEventArgs e)
        {
            String selectedItem = strict_e.SelectedItem.ToString();
            if (selectedItem.Contains("loose"))
                _strict_e = " ";
            else
                _strict_e = "-s";
        }

        private void check_fields_e(object sender, RoutedEventArgs e)
        {
            if (pass_e.Text.Equals("") ||
                img_e.Text.Equals(""))
            {
                System.Windows.Forms.MessageBox.Show(
                    "All input field are required!");
                return;
            }

            String args =
                "-e " + _strict_e + " " + _verbose_e +
                " -f \"" + db_e.Text + "\" " +
                "\"" + pass_e.Text + "\" " +
                "\"" + img_e.Text + "\"";

            call_stide(args, 1);
        }
        // ---> Extract

        // <--- Create
        private void img_c_in_click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
                img_c_in.Text = openFileDialog.FileName;
        }

        private void img_c_out_click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
                img_c_out.Text = openFileDialog.FileName;
        }

        private void db_c_click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
                db_c.Text = openFileDialog.FileName;
        }

        private void verbose_c_set(object sender, RoutedEventArgs e)
        {
            String selectedItem = verbose_c.SelectedItem.ToString();
            if (selectedItem.Contains("silent"))
                _verbose_c = " ";
            else if (selectedItem.Contains("verbose"))
                _verbose_c = "-v";
            else
                _verbose_c = "-d";
        }

        private void strict_c_set(object sender, RoutedEventArgs e)
        {
            String selectedItem = strict_c.SelectedItem.ToString();
            if (selectedItem.Contains("loose"))
                    _strict_c = " ";
            else
                _strict_c = "-s";
        }

        private void check_p(object sender, RoutedEventArgs e)
        {
            _p = "-p";
        }

        private void uncheck_p(object sender, RoutedEventArgs e)
        {
            _p = " ";
        }

        private void check_fields_c(object sender, RoutedEventArgs e)
        {
            if (pass_c.Text.Equals("") || 
                img_c_in.Text.Equals("") || 
                msg.Text.Equals(""))
            {
                System.Windows.Forms.MessageBox.Show(
                    "All input field are required!");
                return;
            }

            String args = 
                "-c " + _strict_c + " " + _p + " " + _verbose_c +
                " -f \"" + db_c.Text + "\" \"" + pass_c.Text + "\" \"" + 
                msg.Text + "\" \"" + img_c_in.Text + "\" " +
                "\"" + img_c_out.Text + "\"";

            call_stide(args, 0);
        }
        // ---> Extract

        // stide caller
        private void call_stide(String args, int mode)
        {
            Process process = new Process();
            process.StartInfo.FileName = "stide.exe";
            process.StartInfo.Arguments = args;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.RedirectStandardOutput = true;
            process.Start();

            // Synchronously read the standard output of the spawned process. 
            StreamReader reader = process.StandardOutput;
            string output = reader.ReadToEnd();

            // Write the redirected output to this application's window.
            if (mode == 0)
                console_c.Text = output;
            else
                console_e.Text = output;
            process.WaitForExit();
            process.Close();
        }
    }
}
