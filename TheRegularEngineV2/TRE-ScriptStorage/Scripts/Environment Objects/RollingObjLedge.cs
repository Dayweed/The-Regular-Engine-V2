namespace TRE
{
	public class RollingObjLedge : Entity
	{
		//public bool isGrounded = true;
		//private Entity Ground;

		//private float moveSpeed = 1.25f;

		public RollingObjLedge()
		{

		}

		public void Start()
		{

		}

		public void Update()
		{
			//// Check if the ledges is no longer being triggered
			//if (Ground == null) return;

			//if (PhysicsSystem.IsTriggerExit(ID, Ground.ID))
			//{
			//    Debug.Log("EXIT");
			//    isGrounded = false;
			//    Ground = null;
			//}
		}

		private void OnTriggerStay(System.UInt64 otherID)
		{
			//Entity other = new Entity(otherID);
			//if (other.CompareTag("Ground"))
			//{
			//    isGrounded = true;
			//    Ground = other;
			//}
		}
	}
}