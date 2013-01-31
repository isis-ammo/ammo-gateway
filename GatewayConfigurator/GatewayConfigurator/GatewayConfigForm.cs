using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using GatewayConfigurator;

namespace GatewayConfigurator
{
    public partial class GatewayConfigForm : Form
    {
        SerialPluginConfig c;
        public GatewayConfigForm()
        {
            InitializeComponent();
            statusLabel.Text = "Loading...";
            try
            {
                c = SerialPluginConfig.loadConfig("C:\\ProgramData\\ammo-gateway\\SerialPluginConfig.json");
                statusLabel.Text = "Config file loaded.";
            }
            catch (System.IO.FileNotFoundException e)
            {
                statusLabel.Text = "Config file not found; using defaults (saving will create a new file).";
                c = new SerialPluginConfig();
            }
            catch (System.Runtime.Serialization.SerializationException e)
            {
                MessageBox.Show(this, "Invalid content was found in the config file.\r\nDefault settings will be used;\r\nsaving will overwrite existing settings.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                statusLabel.Text = "Defaults loaded.";
                c = new SerialPluginConfig();
            }
            catch (Exception e)
            {
                MessageBox.Show(this, "An error occurred while loading:\r\n" + e.ToString() + "\r\n\r\nDefault settings will be used;\r\nsaving will overwrite existing settings.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                statusLabel.Text = "Defaults loaded.";
                c = new SerialPluginConfig();
            }
            bindingSource1.Add(c);
        }

        private void GatewayConfigForm_Load(object sender, EventArgs e)
        {

        }

        private void GatewayConfigForm_Shown(object sender, EventArgs e)
        {
            textBox1.Text = c.ToString();
        }

        private void textBoxValidated(object sender, EventArgs e)
        {
            textBox1.Text = c.ToString();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            c.WriteConfig("C:\\ProgramData\\ammo-gateway\\SerialPluginConfig.json");
        }
    }
}
