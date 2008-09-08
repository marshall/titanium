#ifndef _NSBUILDID_H_
#define _NSBUILDID_H_
/* Build ID file.
*
* If building MOZILLLA_OFFICIAL (release build) NS_BUILD_ID will be updated
* to a current build id. This will be used to determine if we need to 
* re-register components.
*
*/
#define NS_BUILD_ID 2005121221

/* GRE_BUILD_ID - GRE build version identifier
 *
 * If creating a release build (eg, MOZILLA_OFFICIAL is set), then 
 * GRE_BUILD_ID will be updated to contain <milestone>_<build id>.  
 * If building a milestone build (eg, MOZ_MILESTONE_RELEASE is set), then 
 * GRE_BUILD_ID will just contain <milestone>. 
 *
 */
#define GRE_BUILD_ID "1.8_2005121221"

#endif /* _NSBUILDID_H_ */

