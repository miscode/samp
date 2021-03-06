/*  SA:MP Vehicle Functions
 *
 *  (c) Copyright 2005-2006, SA:MP Team
 *
 */

#if defined _vehicles_included
	#endinput
#endif
#define _vehicles_included
#pragma library vehicles

#define CARMODTYPE_SPOILER 0
#define CARMODTYPE_HOOD 1
#define CARMODTYPE_ROOF 2
#define CARMODTYPE_SIDESKIRT 3
#define CARMODTYPE_LAMPS 4
#define CARMODTYPE_NITRO 5
#define CARMODTYPE_EXHAUST 6
#define CARMODTYPE_WHEELS 7
#define CARMODTYPE_STEREO 8
#define CARMODTYPE_HYDRAULICS 9
#define CARMODTYPE_FRONT_BUMPER 10
#define CARMODTYPE_REAR_BUMPER 11
#define CARMODTYPE_VENT_RIGHT 12
#define CARMODTYPE_VENT_LEFT 13

// Vehicle
native CreateVehicle(vehicletype, Float:x, Float:y, Float:z, Float:rotation, color1, color2, respawn_delay);
native DestroyVehicle(vehicleid);
native GetVehiclePos(vehicleid, &Float:x, &Float:y, &Float:z);
native SetVehiclePos(vehicleid, Float:x, Float:y, Float:z);
native GetVehicleZAngle(vehicleid, &Float:z_angle);
native SetVehicleZAngle(vehicleid, Float:z_angle);
native SetVehicleParamsForPlayer(vehicleid,playerid,objective,doorslocked);
native SetVehicleToRespawn(vehicleid);
native LinkVehicleToInterior(vehicleid, interiorid);
native AddVehicleComponent(vehicleid, componentid);
native RemoveVehicleComponent(vehicleid, componentid);
native GetVehicleComponentType(component);
native GetVehicleComponentInSlot(vehicleid, slot);
native ChangeVehicleColor(vehicleid, color1, color2);
native ChangeVehiclePaintjob(vehicleid, paintjobid);
native SetVehicleHealth(vehicleid, Float:health);
native GetVehicleHealth(vehicleid, &Float:health);
native AttachTrailerToVehicle(trailerid, vehicleid);
native DetachTrailerFromVehicle(vehicleid);
native IsTrailerAttachedToVehicle(vehicleid);
native GetVehicleTrailer(vehicleid);
native SetVehicleNumberPlate(vehicleid, numberplate[]);
native GetVehicleModel(vehicleid);
native GetVehicleWeaponRotation(vehicleid, &Float:fX, &Float:fY);

// Virtual Worlds
native SetVehicleVirtualWorld(vehicleid, worldid);
native GetVehicleVirtualWorld(vehicleid);
