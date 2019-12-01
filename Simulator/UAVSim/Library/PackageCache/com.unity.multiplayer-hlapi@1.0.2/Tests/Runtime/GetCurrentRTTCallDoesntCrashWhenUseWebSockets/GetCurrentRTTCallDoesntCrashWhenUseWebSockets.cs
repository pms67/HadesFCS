using System.Collections;
using UnityEngine.TestTools;
using NUnit.Framework;
using UnityEngine;
using UnityEngine.Networking;

#pragma warning disable 618
public class GetCurrentRTTCallDoesntCrashWhenUseWebSockets
{
    [UnityTest]
    [UnityPlatform(RuntimePlatform.WindowsPlayer)]
    [UnityPlatform(RuntimePlatform.LinuxPlayer)]
    [UnityPlatform(RuntimePlatform.OSXPlayer)]
    public IEnumerator GetCurrentRTTCallDoesntCrashWhenUseWebSocketsTest()
    {
        NetworkClient.ShutdownAll();
        NetworkServer.Reset();

        GameObject nmObject = new GameObject();
        NetworkManager nmanager = nmObject.AddComponent<NetworkManager>();
        nmanager.playerPrefab = Resources.Load("GetCurrentRTTCallDoesntCrashWhenUseWebSockets_PlayerPrefab", typeof(GameObject)) as GameObject;
        nmanager.networkAddress = "localhost";
        nmanager.useWebSockets = true;

        LogAssert.Expect(LogType.Error, "the function called has not been supported for web sockets communication");
        nmanager.StartHost();
        yield return null;

        Assert.IsTrue(NetworkServer.active, "Server is not active after StartHost");
        Assert.IsTrue(NetworkClient.active, "Client is not active after StartHost");

        yield return null;
        GameObject player = GameObject.Find("GetCurrentRTTCallDoesntCrashWhenUseWebSockets_PlayerPrefab(Clone)");

        while (!player.GetComponent<UnetPlayerWithGetCurrentRTTCallScript>().isDone)
        {
            yield return null;
        }
        nmanager.StopHost();
        yield return null;
        Assert.IsNull(GameObject.Find("GetCurrentRTTCallDoesntCrashWhenUseWebSockets_PlayerPrefab(Clone)"), "PlayerPrefab(Clone) object should be destroyed after calling StopHost");

        Object.Destroy(nmObject);
    }
}
#pragma warning restore 618
