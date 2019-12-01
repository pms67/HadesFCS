using UnityEngine;
using UnityEngine.TestTools;
using NUnit.Framework;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.Networking;

#pragma warning disable 618
public class NetworkBehaviourCallbacksOrderOnTheHost
{
    public static List<string> expectedListOfCallbacks = new List<string>()
    {
        "OnStartServer",
        "OnStartClient",
        "OnRebuildObservers",
        "OnStartAuthority",
        "OnStartLocalPlayer",
        "Start",
        "OnSetLocalVisibility",
        "OnSetLocalVisibility"
    };

    //[KnownFailure(855941, "OnSetLocalVisibility callback should appear only once ")]
    [UnityTest]
    public IEnumerator CallbacksOrderInNetworkBehaviourOnTheHostIsCorrect()
    {
        NetworkClient.ShutdownAll();
        NetworkServer.Reset();

        GameObject nmObject = new GameObject();
        NetworkManager nmanager = nmObject.AddComponent<NetworkManager>();
        nmanager.playerPrefab = Resources.Load("PlayerCallbacksOrderOnTheHost_PlayerPrefab", typeof(GameObject)) as GameObject;

        Assert.IsNotNull(nmanager.playerPrefab, "Player prefab field is not set on NetworkManager");

        nmanager.StartHost();
        yield return null;

        Assert.IsTrue(NetworkServer.active, "Server is not active after StartHost");
        Assert.IsTrue(NetworkClient.active, "Client is not active after StartHost");
        yield return null;
        GameObject player = GameObject.Find("PlayerCallbacksOrderOnTheHost_PlayerPrefab(Clone)");
        yield return null;

        while (!player.GetComponent<PlayerCallbacksOrderOnTheHostScript>().isDone)
        {
            yield return null;
        }
        nmanager.StopHost();
        CollectionAssert.AreEqual(expectedListOfCallbacks, player.GetComponent<PlayerCallbacksOrderOnTheHostScript>().actualListOfCallbacks, "Wrong order of callbacks or some callback is missing");
        Object.Destroy(nmObject);
    }
}
#pragma warning restore 618
