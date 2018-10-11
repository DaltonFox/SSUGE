import ssuge
import math

class PlayerTank(ssuge.GameObject):
	def onCreate(self, args):
		self.bullets = args[0]
		self.mSpeed = 155
		self.mKills = 0
		self.mFlipped = False
		self.shot = 0
		self.health = 100
		ssuge.registerInputListener(self)
		self.canShoot = True
		self.reload = 0.15

		self.setScale(20, 20, 20)
		self.createMeshComponent("..\\Media\\models\\anubisTank.mesh")
		
		self.mCameraRotater = ssuge.GameObject("pyobjects", "camera_rot")
		self.addChild(self.mCameraRotater)
		self.mRotationAngle = -90
		self.mYawAngle = 180
		
		self.mCamera = ssuge.GameObject("pyobjects", "camera")
		self.mCameraRotater.addChild(self.mCamera)
		self.mCamera.setPositionParent(0, 0, 600)
		self.mCamera.createCameraComponent(0)

		ssuge.setAudioListener(self.mCamera)
		
		self.mTurret = ssuge.GameObject("pyobjects", "turret")
		self.mTurret.createMeshComponent("..\\Media\\models\\anubisTurret.mesh")
		self.mTurret.setScale(20, 20, 20)
		self.addChild(self.mTurret)
		
		ssuge.setMouseVisibility(True)	

		ggroup = "gui_group"
		ssuge.createGroup(ggroup)
		self.mTimeDisplay = ssuge.GameObject(ggroup, "timer")
		self.mTimeDisplay.createGUIComponent("text", 0.3, 0.01, 0.1, 0.05)
		self.mTimeDisplay.setGUIColor(0.0, 0.0, 0.0,   0.3, 0.3, 0.3)
		self.mTimeDisplay.setGUIVisible(True)
		self.mTime = 0.0

		self.mLogo = ssuge.GameObject(ggroup, "logo")
		self.mLogo.createGUIComponent("image", 0.0, 0.0, 1.0, 1.0)
		self.mLogo.setGUIMaterial("Logo")
		self.mLogo.setGUIVisible(True)

		self.mGameMusic = ssuge.GameObject(ggroup, "main-music")
		self.mGameMusic.createSoundComponent("..\\Media\\sounds\\EgyptianTech.ogg", False)
		self.mGameMusic.setSoundLooping(True)
		self.mGameMusic.playSoundComponent()
		self.mouse_angle = 0
	
	def onUpdate(self, args):
		dt = args[0]
		if self.health <= 0:
			ssuge.shutdown()
		move_axes = [ssuge.getAxis("horizontal", 0), ssuge.getAxis("vertical", 0)]		
		self.translateLocal(0, 0, self.mSpeed * move_axes[1] * dt)
		self.rotateWorld(-move_axes[0] * 90 * dt, 0, 1, 0)

		camera_axes = [ssuge.getAxis("horizontal", 1), ssuge.getAxis("vertical", 1)]
		self.mRotationAngle -= camera_axes[1] * 30 * dt
		self.mYawAngle += camera_axes[0] * 90 * dt
		if self.mRotationAngle < -80:
			self.mRotationAngle = -80
		if self.mRotationAngle > 0:
			self.mRotationAngle = 0
		self.mCameraRotater.setOrientation(self.mRotationAngle, 1, 0, 0)
		self.mCameraRotater.rotateWorld(self.mYawAngle, 0, 1, 0)

		x, y, z = self.getWorldPosition()
		if z > 680:
			self.setPositionWorld(x, y, 680)
		elif z < -680:
			self.setPositionWorld(x, y, -680)

		if x > 880:
			self.setPositionWorld(880, y, z)
		elif x < -880:
			self.setPositionWorld(-880, y, z)

		tank_pos = [400, 300]
		mouse_pos = ssuge.getMouseNormalized()
		self.mouse_angle = math.degrees(math.atan2(math.radians(tank_pos[0] - mouse_pos[0]), math.radians(tank_pos[1] - mouse_pos[1])))
		self.mTurret.setOrientation(self.mouse_angle, 0, 1, 0)
		
		self.mTime += 1 * dt
		self.mTimeDisplay.updateGUIText("Total Kills: " + str(self.mKills) + " Time Alive: " + str(round(self.mTime, 1)))
		if self.canShoot == False:
			self.reload -= 1 * dt
			if self.reload <= 0:
				self.canShoot = True
				self.reload = 0.2

	def onAction(self, args):
		action = args[0]
		is_start = args[1]
		if action == "menu":
			ssuge.shutdown()
		elif action == "attack":
			if self.canShoot == True:
				pos = self.getWorldPosition()
				self.shot += 1
				a = self.mRotationAngle
				b = self.mouse_angle
				result_angle = math.cos(a) * math.cos(b)
				result_angle += math.sin(a) * math.sin(b)
				self.bullets.contents.append(ssuge.Bullet("pyobjects", "bullet" + str(self.shot), self.mouse_angle, pos[0], pos[2]))
				self.canShoot = False

class Pod(ssuge.GameObject):
	def onCreate(self, args):
		x = args[0]
		y = args[1]
		self.enemy = args[2]
		self.size = 5
		self.setScale(self.size, self.size, self.size)
		self.createMeshComponent("..\\Media\\models\\anubisPod.mesh")
		self.translateLocal(x, 6, y)
		self.mMouseMode = True
		self.speed = ssuge.getRandomFloat(60, 80)
		self.speedOT = ssuge.getRandomFloat(2.5, 6)
		
	def reset(self):
		self.speed = ssuge.getRandomFloat(60, 80)
		self.speedOT = ssuge.getRandomFloat(2.5, 6)
		tank_pos = self.enemy.getWorldPosition()
		if (ssuge.getRandomInt(0, 2)):
			x = ssuge.getRandomFloat(-800, 800)
			if x > 0:
				x += tank_pos[0]
			else:
				x -= tank_pos[0]
			if (ssuge.getRandomInt(0, 2)):
				y = tank_pos[2] + 800
			else:
				y = tank_pos[2] - 800
		else:
			y = ssuge.getRandomFloat(-800, 800)
			if y > 0:
				y += tank_pos[2]
			else:
				y -= tank_pos[2]
			if (ssuge.getRandomInt(0, 2)):
				x = tank_pos[0] + 800
			else:
				x = tank_pos[0] - 800

		self.translateLocal(x, 0, y)

	def onUpdate(self, args):
		dt = args[0]

		target = self.enemy.getWorldPosition()
		pos = self.getWorldPosition()
		towards = ssuge.MoveTowards(pos, target, self.speed * dt)
		self.setPositionWorld(towards[0], towards[1], towards[2])
		self.speed += self.speedOT * dt
		
		distance = math.sqrt(math.pow(target[0] - pos[0], 2) + math.pow(target[1] - pos[1], 2) + math.pow(target[2] - pos[2], 2))
		if distance < self.size * 5:
			self.enemy.health -= 20
			self.reset()

		for bullet in self.enemy.bullets.contents:
			target = bullet.getWorldPosition()
			distance = math.sqrt(math.pow(target[0] - pos[0], 2) + math.pow(target[1] - pos[1], 2) + math.pow(target[2] - pos[2], 2))
			if distance < self.size * 3:
				self.enemy.mKills += 1
				self.reset()


class Pyramid(ssuge.GameObject):
	def onCreate(self, args):
		size = ssuge.getRandomFloat(150, 160)
		rotate = ssuge.getRandomFloat(0, 360)
		self.setScale(size, size, size)
		self.createMeshComponent("..\\Media\\models\\anubisPyramid.mesh")
		self.translateLocal(args[0], -15, args[1])
		self.setOrientation(rotate, 0, 1, 0)

class Terrain(ssuge.GameObject):
	def onCreate(self, args):
		self.setScale(825, 1, 825)
		self.createMeshComponent("..\\Media\\models\\anubisTerrain.mesh")
		self.translateLocal(0, -15, 0)

class ItemContainer(ssuge.GameObject):
	def onCreate(self, args):
		self.contents = []

class Bullet(ssuge.GameObject):
	def onCreate(self, args):
		self.mDirection = args[0]
		self.translateLocal(args[1], 4, args[2])
		self.mSpeed = 800
		self.setScale(0.5, 0.5, 4)
		self.createMeshComponent("..\\Media\\models\\anubisPod.mesh")
		self.setOrientation(self.mDirection, 0, 1, 0)

	def onUpdate(self, args):
		dt = args[0]
		self.translateLocal(0, 0, self.mSpeed * 1 * dt)
	

ssuge.PlayerTank = PlayerTank
ssuge.Pod = Pod
ssuge.Pyramid = Pyramid
ssuge.Terrain = Terrain
ssuge.ItemContainer = ItemContainer
ssuge.Bullet = Bullet