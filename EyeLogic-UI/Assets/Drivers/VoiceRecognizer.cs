using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Windows.Speech;
using System.Linq;

public class VoiceRecognizer : MonoBehaviour {


    KeywordRecognizer keywordRecognizer;
    Dictionary<string, System.Action> keywords = new Dictionary<string, System.Action>();

    private static bool dragState; 

	// Use this for initialization
	private void Start ()
    {
        dragState = false;
        keywords.Add("click", () =>
        {
            Debug.Log("Click Command");
            MouseControl.handle.click();

        });
        keywords.Add("drag", () =>
        {
            if(dragState)
            {
                Debug.Log("Stop Drag Command");
                MouseControl.handle.stopdrag();
            }
            else
            {
                Debug.Log("Start Drag Command");
                MouseControl.handle.startdrag();
            }
            dragState = !dragState;

        });
        keywords.Add("double", () =>
        {
            Debug.Log("Double Click Command");
            MouseControl.handle.click();
            MouseControl.handle.click();
        });
        keywords.Add("right", () =>
        {
            Debug.Log("Right Command");
            MouseControl.handle.rightclick();
            
        });
        keywords.Add("stop", () =>
        {
            Application.Quit();

        });
        keywords.Add("setup", () =>
        {
            //MouseControl.handle.click();

        });
        keywordRecognizer = new KeywordRecognizer(keywords.Keys.ToArray());
        keywordRecognizer.OnPhraseRecognized += KeywordRecognizerOnPhraseRecognized;
        keywordRecognizer.Start();
	}

    private void KeywordRecognizerOnPhraseRecognized(PhraseRecognizedEventArgs args)
    {
        System.Action keywordAction;
        if(keywords.TryGetValue(args.text, out keywordAction))
        {
            keywordAction.Invoke();
        }
    }
}
