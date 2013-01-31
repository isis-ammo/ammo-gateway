using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;

namespace GatewayConfigurator
{
    [DataContract]
    public class SerialPluginConfig
    {
        public SerialPluginConfig()
        {
            this.ListenPort = "COM1";
            this.GpsPort = "COM2";
            this.SendEnabled = false;
            this.BaudRate = 9600;
            this.SlotDuration = 750;
            this.SlotNumber = 15;
            this.NumberOfSlots = 16;
            this.TransmitDuration = 500;
            this.GpsTimeOffset = 0;
            this.PliRelayEnabled = true;
            this.PliRelayPerCycle = 4;
            this.PliSendFrequency = 2;
            this.PliRelayNodeName = "base";
            this.RangeScale = 4;
            this.TimeScale = 4;
        }

        public static SerialPluginConfig loadConfig(String configFilePath)
        {
            DataContractJsonSerializer jsonSerializer = new DataContractJsonSerializer(typeof(SerialPluginConfig));

            SerialPluginConfig newConfig = null;

            using (Stream configStream = File.Open(configFilePath, FileMode.Open))
            {
                newConfig = (SerialPluginConfig)jsonSerializer.ReadObject(configStream);
            }

            return newConfig;
        }

        public void WriteConfig(String configFilePath)
        {
            DataContractJsonSerializer jsonSerializer = new DataContractJsonSerializer(typeof(SerialPluginConfig));

            using(Stream configStream = File.Open(configFilePath, FileMode.Create))
            {
                jsonSerializer.WriteObject(configStream, this);
            }
        }

        public override String ToString()
        {
            StringBuilder sb = new StringBuilder();

            PropertyInfo[] properties = this.GetType().GetProperties();

            foreach (PropertyInfo p in properties)
            {
                sb.AppendFormat("{0}: {1} ({2})\r\n", p.Name, p.GetValue(this, null), p.PropertyType.Name);
            }

            return sb.ToString();
        }

        [DataMember(Name = "listenPort")]
        public string ListenPort { get; set; }

        [DataMember(Name = "gpsPort")]
        public string GpsPort { get; set; }

        [DataMember(Name = "sendEnabled")]
        public bool SendEnabled { get; set; }

        [DataMember(Name = "baudRate")]
        public int BaudRate { get; set; }

        [DataMember(Name = "slotDuration")]
        public int SlotDuration { get; set; }

        [DataMember(Name = "slotNumber")]
        public int SlotNumber { get; set; }

        [DataMember(Name = "numberOfSlots")]
        public int NumberOfSlots { get; set; }

        [DataMember(Name = "transmitDuration")]
        public int TransmitDuration { get; set; }

        [DataMember(Name = "gpsTimeOffset")]
        public int GpsTimeOffset { get; set; }

        [DataMember(Name = "pliRelayPerCycle")]
        public int PliRelayPerCycle { get; set; }

        [DataMember(Name = "pliRelayEnabled")]
        public bool PliRelayEnabled { get; set; }

        [DataMember(Name = "pliSendFrequency")]
        public int PliSendFrequency { get; set; }

        [DataMember(Name = "pliRelayNodeName")]
        public string PliRelayNodeName { get; set; }

        [DataMember(Name = "rangeScale")]
        public int RangeScale { get; set; }

        [DataMember(Name = "timeScale")]
        public int TimeScale { get; set; }
    }
}
