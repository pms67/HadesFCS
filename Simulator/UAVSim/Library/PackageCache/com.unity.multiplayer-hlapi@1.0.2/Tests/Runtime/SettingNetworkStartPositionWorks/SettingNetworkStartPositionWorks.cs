using System.Collections;
using NUnit.Framework;
using UnityEngine.TestTools;
using UnityEngine;
using UnityEngine.Networking;

#pragma warning disable 618
public class SettingNetworkStartPositionWorks
{
    public static Vector3 startpos = new Vector3(1.4f, 6.3f, 6.23f);

    public class TestNetworkManagerStartPos : NetworkManager
    {
        public bool isDone = false;

        public override void OnServerAddPlayer(NetworkConnection conn, short playerControllerId)
        {
            base.OnServerAddPlayer(conn, playerControllerId);
            StringAssert.IsMatch(conn.playerControllers[0].gameObject.transform.position.ToString(), startpos.ToString());
            isDone = true;
        }
    }

    [UnityTest]
    public IEnumerator SettingNetworkStartPositionWorksTest()
    {
        NetworkServer.Reset();
        NetworkClient.ShutdownAll();

        GameObject nmObject = new GameObject();
        TestNetworkManagerStartPos nmanager = nmObject.AddComponent<TestNetworkManagerStartPos>();
        nmanager.playerPrefab = Resources.Load("CleanPlayerPrefab", typeof(GameObject)) as GameObject;
        nmanager.networkAddress = "localhost";

        var start = new GameObject();
        start.transform.position = startpos;
        start.AddComponent<NetworkStartPosition>();

        nmanager.StartServer();
        nmanager.StartClient();
        yield return null;

        Assert.IsTrue(NetworkServer.active, "Server is not started");
        Assert.IsTrue(NetworkClient.active, "Client is not started");
        yield return null;

        while (!nmanager.isDone)
        {
            yield return null;
        }

        NetworkManager.singleton.StopServer();
        NetworkManager.singleton.StopClient();

        Object.Destroy(nmObject);
    }
}
#pragma warning restore 618
