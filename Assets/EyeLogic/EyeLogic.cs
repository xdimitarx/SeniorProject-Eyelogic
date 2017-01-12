using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.UI;


public class EyeLogic : MonoBehaviour {

    [DllImport("EyeLogic.dll")]
    public static extern int initEyeLogic();

    public GameObject theButtonOverlord;
	public Button theButton;
	// Use this for initialization


	void Start () {
		theButton.onClick.AddListener (destroyMe);
	}
	
	// Update is called once per frame
	void Update () {
		
	}

	void destroyMe()
	{
        Debug.Log(initEyeLogic());
		Destroy (theButtonOverlord);
	}

}


