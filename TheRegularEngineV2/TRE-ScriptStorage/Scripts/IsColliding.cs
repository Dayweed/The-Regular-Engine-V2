namespace TRE
{
	class IsColliding : Entity
	{
		public void OnCollisionEnter(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			if (other.CompareTag("Red") || other.CompareTag("Blue"))
				UpdateParent(true);
		}

		public void OnCollisionStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			if (other.CompareTag("Red") || other.CompareTag("Blue"))
				UpdateParent(true);
		}

		public void OnCollisionExit(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			if (other.CompareTag("Red") || other.CompareTag("Blue"))
				UpdateParent(false);
		}

		void UpdateParent(bool state)
		{
			parenting.parent.GetComponent<OnOffPlatform>().isCollidingWithPlayer = state;
		}
	}
}
