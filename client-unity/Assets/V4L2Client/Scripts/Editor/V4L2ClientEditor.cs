using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

[CustomEditor(typeof(V4L2Client))]
public class V4L2ClientEditor : Editor
{
	public override void OnInspectorGUI()
	{
		var tar = target as V4L2Client;
		DrawDefaultInspector();

		EditorGUILayout.BeginVertical();

		if(GUILayout.Button("exit"))
		{
			tar.RemoteExit();
		}

		if(GUILayout.Button("start"))
		{
			tar.Connect();
			tar.RemoteCapture(true);
		}

		if(GUILayout.Button("stop"))
		{
			tar.RemoteCapture(false);
			tar.Disconnect();
		}

		EditorGUILayout.EndVertical();
	}

}