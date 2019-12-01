using System.Collections;
using System.Collections.Generic;
using NUnit.Framework;
using UnityEngine.TestTools;
using UnityEngine;
using UnityEngine.Networking;

#pragma warning disable 618
public class NetworkManagerCallbacksOrderOnTheHost
{
    public static List<string> resultListOfCallbacks = new List<string>()
    {
        "OnStartHost",
        "OnStartServer",
        "OnServerConnect",
        "OnStartClient",
        "OnServerReady",
        "OnServerAddPlayer",
        "OnClientConnect",
        "OnStopHost",
        "OnStopServer",
        "OnStopClient"
    };

    [UnityTest]
    public IEnumerator CallbacksOrderInNetworkManagerOnTheHostIsCorrect()
    {
        NetworkServer.Reset();
        NetworkClient.ShutdownAll();

        GameObject nmObject = new GameObject();
        CustomNetworkManagerWithCallbacks nmanager = nmObject.AddComponent<CustomNetworkManagerWithCallbacks>();
        nmanager.playerPrefab = Resources.Load("CleanPlayerPrefab", typeof(GameObject)) as GameObject;

        yield return null;
        Assert.IsNotNull(nmanager.playerPrefab, "Player prefab field is not set on NetworkManager");

        nmanager.StartHost();
        yield return null;

        Assert.IsTrue(NetworkServer.active, "Server is not active after StartHost");
        Assert.IsTrue(NetworkClient.active, "Client is not active after StartHost");
        yield return null;

        while (!nmanager.isStartHostPartDone)
        {
            yield return null;
        }

        nmanager.StopHost();
        while (!nmanager.isStopHostPartDone)
        {
            yield return null;
        }

        CollectionAssert.AreEqual(resultListOfCallbacks, nmanager.actualListOfCallbacks, "Wrong order of callbacks or some callback is missing");
        Object.Destroy(nmObject);
    }
}
#pragma warning restore 618
