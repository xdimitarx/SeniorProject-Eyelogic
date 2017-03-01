using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class DisplayCam : MonoBehaviour {

    public RawImage toPrint;
	
	// Update is called once per frame
	void FixedUpdate () {
        Texture capture = WebcamCapture.handle.GetUnityImage();
        toPrint.material.mainTexture = capture;
	}
}
