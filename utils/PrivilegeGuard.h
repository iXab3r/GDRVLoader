#pragma once
#include "../global.h"

class PrivilegeGuard
{
public:
	explicit PrivilegeGuard(ULONG privilege)
		: _privilege(privilege), _wasEnabled(false), _adjusted(false)
	{
		_status = RtlAdjustPrivilege(
			_privilege,
			TRUE,   // Enable
			FALSE,  // Process-wide
			&_wasEnabled);

		if (NT_SUCCESS(_status))
		{
			_adjusted = true;
		}
		else
		{
			Log("WARNING: Failed to enable privilege %lu. NTSTATUS: 0x%08X\n", _privilege, _status);
		}
	}

	NTSTATUS Status() const { return _status; }

	~PrivilegeGuard()
	{
		if (_adjusted && !_wasEnabled)
		{
			// Only restore if we actually changed it
			BOOLEAN ignored;
			NTSTATUS status = RtlAdjustPrivilege(
				_privilege,
				FALSE,  // Disable
				FALSE,
				&ignored);

			if (!NT_SUCCESS(status))
			{
				Log("WARNING: Failed to restore privilege %lu. NTSTATUS: 0x%08X\n", _privilege, status);
			}
		}
	}

private:
	ULONG _privilege;
	BOOLEAN _wasEnabled;
	bool _adjusted;
	NTSTATUS _status;
};
