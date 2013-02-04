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
    public class GatewayConfig
    {
        public GatewayConfig()
        {
            this.GatewayInterface = "127.0.0.1";
            this.GatewayAddress = "127.0.0.1";
            this.GatewayPort = 12475;
            this.CrossGatewayId = "MainGateway";
            this.CrossGatewayServerInterface = "127.0.0.1";
            this.CrossGatewayServerPort = 47543;
            this.CrossGatewayParentAddress = "";
            this.CrossGatewayParentPort = 47543;
        }

        public static GatewayConfig loadConfig(String configFilePath)
        {
            DataContractJsonSerializer jsonSerializer = new DataContractJsonSerializer(typeof(GatewayConfig));

            GatewayConfig newConfig = null;

            using (Stream configStream = File.Open(configFilePath, FileMode.Open))
            {
                newConfig = (GatewayConfig)jsonSerializer.ReadObject(configStream);
            }

            return newConfig;
        }

        public void WriteConfig(String configFilePath)
        {
            DataContractJsonSerializer jsonSerializer = new DataContractJsonSerializer(typeof(GatewayConfig));

            using (Stream configStream = File.Open(configFilePath, FileMode.Create))
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

        [DataMember(Name = "GatewayInterface")]
        public string GatewayInterface { get; set; }

        [DataMember(Name = "GatewayAddress")]
        public string GatewayAddress { get; set; }

        [DataMember(Name = "GatewayPort")]
        public int GatewayPort { get; set; }

        [DataMember(Name = "CrossGatewayId")]
        public string CrossGatewayId { get; set; }

        [DataMember(Name = "CrossGatewayServerInterface")]
        public string CrossGatewayServerInterface { get; set; }

        [DataMember(Name = "CrossGatewayServerPort")]
        public int CrossGatewayServerPort { get; set; }

        [DataMember(Name = "CrossGatewayParentAddress")]
        public string CrossGatewayParentAddress { get; set; }

        [DataMember(Name = "CrossGatewayParentPort")]
        public int CrossGatewayParentPort { get; set; }
    }
}
