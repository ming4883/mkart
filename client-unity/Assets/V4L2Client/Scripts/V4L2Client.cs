using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net.Sockets;

public class V4L2Client : MonoBehaviour
{
	public bool debug = true;
	public string hostName = "192.168.1.82";
	public int port = 8080;

	private TcpClient m_TcpClient = null;

	private List<byte> m_Received = new List<byte>();
	private List<byte> m_Message = null;
	private byte[] m_ReadBuffer = new byte[1024 * 1024];
	private byte[] m_4CC = new byte[4];

	public Texture2D texture;

	public struct GPIOValues
	{
		public uint bank;
		public uint values;
	}

	public event System.Action TextureChanged;
	public event System.Action<GPIOValues> GPIOValuesReceived;

	// Use this for initialization
	void Start ()
	{
		texture = new Texture2D(2, 2);
	}
	
	// Update is called once per frame
	void Update ()
	{
		RecieveMessage();
	}

	public void Connect()
	{
		try
		{
			m_TcpClient = new TcpClient(hostName, port);
		}
		catch(System.Exception err)
		{
			Debug.LogErrorFormat("{0}", err);
			m_TcpClient = null;
		}
	}

	public void Disconnect()
	{
		if (null == m_TcpClient)
			return;

		m_TcpClient.Close();
		m_TcpClient = null;
		m_Received.Clear();
		m_Message.Clear();
	}

	public bool RemoteCommand(string i_Cmd)
	{
		if (null == m_TcpClient || !m_TcpClient.Connected)
			return false;

		var bytes = System.Text.Encoding.UTF8.GetBytes(i_Cmd);
		m_TcpClient.GetStream().Write(bytes, 0 , bytes.Length);

		return true;
	}

	public void RemoteExit()
	{
		RemoteCommand("exit();");
	}

	public void RemoteCapture(bool i_Enabled)
	{
		string msg_t = "v4l2_set_capturing(true);";
		string msg_f = "v4l2_set_capturing(false);";
		RemoteCommand(i_Enabled ? msg_t : msg_f);
	}

	public void RemoteGPIOSetModeIn(uint i_GPIONum)
	{
		string msg = string.Format("gpio_pin_mode_in({0});", i_GPIONum);
		RemoteCommand(msg);
	}

	public void RemoteGPIOSetModeOut(uint i_GPIONum)
	{
		string msg = string.Format("gpio_pin_mode_out({0});", i_GPIONum);
		RemoteCommand(msg);
	}

	public void RemoteGPIOSet_0_31_Low(uint i_GPIOBitMask)
	{
		string msg = string.Format("gpio_set_0_31_low({0});", i_GPIOBitMask);
		RemoteCommand(msg);
	}

	public void RemoteGPIOSet_0_31_High(uint i_GPIOBitMask)
	{
		string msg = string.Format("gpio_set_0_31_high({0});", i_GPIOBitMask);
		RemoteCommand(msg);
	}

	public void RemoteGPIOGet_0_31()
	{
		string msg = "gpio_get_0_31();";
		RemoteCommand(msg);
	}

	void RecieveMessage()
	{
		if (null == m_TcpClient || !m_TcpClient.Connected)
			return;

		
		int avil = m_TcpClient.Available;
		if (avil > 0)
		{
			var stm = m_TcpClient.GetStream();
			var actual = stm.Read(m_ReadBuffer, 0, m_ReadBuffer.Length);

			for(int i = 0; i < actual; ++i)
			{
				m_Received.Add(m_ReadBuffer[i]);
			}
		}

		if (m_Received.Count < 4)
			return;

		var msgLen = (int)System.BitConverter.ToUInt32(m_Received.ToArray(), 0);

		if (0 == msgLen)
		{
			m_Received.RemoveRange(0, 4);
			
			if (debug)
				Debug.LogFormat("Keep alive recieved, remain {0} bytes", m_Received.Count);
		}
		else
		{
			if (m_Received.Count >= (msgLen + 4))
			{
				m_4CC[0] = m_Received[4];
				m_4CC[1] = m_Received[5];
				m_4CC[2] = m_Received[6];
				m_4CC[3] = m_Received[7];
				
				m_Message = m_Received.GetRange(8, (int)msgLen - 4);
				m_Received.RemoveRange(0, 4 + msgLen);
				
				if (debug)
				{
					Debug.LogFormat("Message {0} bytes recieved, remain {1} bytes", msgLen, m_Received.Count);
					Debug.LogFormat("4CC = {0}{1}{2}{3}", m_4CC[0], m_4CC[1], m_4CC[2], m_4CC[3]);
				}

				if (Is4CC('J', 'P', 'E', 'G'))
				{
					OnReceiveJPEG();
				}

				if (Is4CC('G', 'P', 'I', 'O'))
				{
					OnReceiveGPIO();
				}
			}
		}
	}

	bool Is4CC(char i_A, char i_B, char i_C, char i_D)
	{
		return m_4CC[0] == i_A && m_4CC[1] == i_B && m_4CC[2] == i_C && m_4CC[3] == i_D;
	}

	void OnReceiveJPEG()
	{
		if(!ImageConversion.LoadImage(texture, m_Message.ToArray(), false))
		{
			if (debug)
				Debug.LogError("Failed to load JPEG from message");
		}

		if (null != TextureChanged)
			TextureChanged.Invoke();
	}

	void OnReceiveGPIO()
	{
		GPIOValues gpioVals = new GPIOValues();

		var msg = m_Message.ToArray();

		gpioVals.bank = System.BitConverter.ToUInt32(msg, 0);
		gpioVals.values = System.BitConverter.ToUInt32(msg, 4);

		if (null != GPIOValuesReceived)
			GPIOValuesReceived.Invoke(gpioVals);
	}
}
