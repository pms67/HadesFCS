using System.Collections;
using NUnit.Framework;
using UnityEngine.TestTools;
using UnityEngine;
using UnityEngine.Networking;

#pragma warning disable 618
public class AuthorityOnSpawnedObjectsIsCorrect
{
    public static bool isTestDone = false;

    [UnityTest]
    public IEnumerator AuthorityOnSpawnedObjectsIsCorrectTest()
    {
        NetworkServer.Reset();
        NetworkClient.ShutdownAll();

        GameObject nmObject = new GameObject();
        NetworkManager nmanager = nmObject.AddComponent<NetworkManager>();
        nmanager.playerPrefab = Resources.Load("PlayerWithAuthPrefab", typeof(GameObject)) as GameObject;
        nmanager.spawnPrefabs.Add(Resources.Load("NoAuthObjPrefab", typeof(GameObject)) as GameObject);
        nmanager.spawnPrefabs.Add(Resources.Load("AuthObjPrefab", typeof(GameObject)) as GameObject);

        Assert.IsNotNull(nmanager.playerPrefab, "Player prefab field is not set on NetworkManager");
        nmanager.StartHost();
        yield return null;

        Assert.IsTrue(NetworkServer.active, "Server is not active after StartHost");
        Assert.IsTrue(NetworkClient.active, "Client is not active after StartHost");

        while (!isTestDone)
        {
            yield return null;
        }

        nmanager.StopHost();
        Object.Destroy(nmObject);
    }
}
#pragma warning restore 618
