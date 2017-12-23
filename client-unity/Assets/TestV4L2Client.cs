using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class TestV4L2Client : MonoBehaviour {

	public V4L2Client m_Client;
	public RawImage m_RawImage;

	public Text m_GPIOBank1;
	public Text m_GPIOBank2;

	public int GpioNumber = 21;

	// Use this for initialization
	void Start ()
	{
		if (null != m_Client)
		{
			m_Client.TextureChanged += () =>
			{
				if (null != m_RawImage)
				{
					m_RawImage.texture = m_Client.texture;
				}
			};

			m_Client.GPIOValuesReceived += (i_Vals) =>
			{
				Text txt = (i_Vals.bank == 1) ? m_GPIOBank1 : m_GPIOBank2;
				if (null != txt)
				{
					txt.text = ToBin(i_Vals.values, 32);
				}
			};
		}
	}

	public static string ToBin(uint value, int len)
	{
		return (len > 1 ? ToBin(value >> 1, len - 1) : null) + "01"[(int)(value & 1)];
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

	public void StartStreaming()
	{
		m_Client.Connect();
		m_Client.RemoteCapture(true);

		m_RawImage.texture = m_Client.texture;
	}

	public void StopStreaming()
	{
		m_Client.RemoteCapture(false);
		m_Client.Disconnect();

		m_RawImage.texture = null;
	}

	public void GPIOTestHigh()
	{
		m_Client.RemoteGPIOSetModeOut((uint)GpioNumber);
		m_Client.RemoteGPIOSet_0_31_High((uint)(1 << GpioNumber));
		m_Client.RemoteGPIOGet_0_31();
	}
	public void GPIOTestLow()
	{
		m_Client.RemoteGPIOSetModeOut((uint)GpioNumber);
		m_Client.RemoteGPIOSet_0_31_Low((uint)(1 << GpioNumber));
		m_Client.RemoteGPIOGet_0_31();
	}
}
