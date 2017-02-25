using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public  class WebcamCapture : MonoBehaviour {

    public static WebcamCapture handle;

    private WebCamTexture webcamTexture;
    private Color32[] data;
     
	// Use this for initialization
	private void Start () {
        handle = this;
        webcamTexture = new WebCamTexture();
        webcamTexture.Play();
        data = new Color32[webcamTexture.width * webcamTexture.height];
    }
	
	// Update is called once per frame
	public Color32[] GetRawImage ()
    {
        webcamTexture.GetPixels32(data);
        return data;
	}

    public Texture GetUnityImage()
    {
        return webcamTexture;
    }
}
