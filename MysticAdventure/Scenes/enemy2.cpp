extends CharacterBody2D

signal died

@export var speed: float = 40.0
@export var max_health: int = 20
@export var life: int = 20
@export var damage: int = 1
@export var follow_range: float = 500.0
@export var attack_range: float = 45.0

var player: Node2D = null
var is_stunned := false
var is_attacking := false
var is_dead := false
var facing_direction := 1

# efeitos
var is_frozen := false
var is_burning := false
var is_marked := false

@export var burn_damage: int = 2

func _ready():
	add_to_group("enemies")
	player = get_tree().get_first_node_in_group("player")
	life = max_health

func _physics_process(delta):
	if is_dead:
		return
		
	if player == null:
		velocity.x = 0
		play_if_not("idle")
		move_and_slide()
		return
		
	if is_frozen:
		velocity.x = 0
		play_if_not("idle")
		move_and_slide()
		return
		
	if is_stunned:
		velocity.x = 0
		play_if_not("idle")
		move_and_slide()
		return
		
	if is_attacking:
		velocity.x = 0
		play_if_not("attack")
		move_and_slide()
		return
		
	var distance_x = player.global_position.x - global_position.x
	var distance_abs = abs(distance_x)
	
	if distance_x > 0:
		facing_direction = 1
	else:
		facing_direction = -1
		
	$AnimatedSprite2D.flip_h = facing_direction < 0
	
	if distance_abs <= attack_range:
		start_attack()
	elif distance_abs <= follow_range:
		velocity.x = speed * facing_direction
		play_if_not("run")
	else:
		velocity.x = 0
		play_if_not("idle")

	move_and_slide()

func start_attack():
	if is_attacking or is_dead or is_frozen:
		return
		
	is_attacking = true
	velocity.x = 0
	play_if_not("attack")
	$AttackTimer.start()
	
func deal_damage_to_player():
	if player != null and player.has_method("take_damage"):
		if abs(player.global_position.x - global_position.x) <= attack_range + 10:
			player.take_damage(damage)

func take_hit(amount: int, effect_type: String = "", effect_duration: float = 0.0):
	if is_dead:
		return
	
	var final_damage = amount
	
	if is_marked and effect_type != "mark":
		final_damage += 2
		is_marked = false
		$MarkTimer.stop()
		print("Marca consumida! Dano aumentado.")
	
	take_damage(final_damage)
	
	match effect_type:
		"freeze":
			apply_freeze(effect_duration)
		"burn":
			apply_burn(effect_duration)
		"mark":
			apply_mark(effect_duration)

func take_damage(amount: int):
	if is_dead:
		return
		
	life -= amount
	if life < 0:
		life = 0
		
	print("inimigo tomou dano: ", amount)
	print("vida restante: ", life)
	
	if life <= 0:
		die()
		return
		
	is_stunned = true
	is_attacking = false
	velocity.x = 0
	$StunTimer.start()
	play_if_not("idle")

func apply_freeze(duration: float):
	if is_dead:
		return
	
	is_frozen = true
	is_attacking = false
	velocity = Vector2.ZERO
	$FreezeTimer.start(duration)
	print("Inimigo congelado por ", duration, " segundos")

func apply_burn(duration: float):
	if is_dead:
		return
	
	is_burning = true
	$BurnTimer.start(duration)
	$BurnTickTimer.start(1.0)
	print("Inimigo queimando por ", duration, " segundos")

func apply_mark(duration: float):
	if is_dead:
		return
	
	is_marked = true
	$MarkTimer.start(duration)
	print("Inimigo marcado por ", duration, " segundos")

func die():
	is_dead = true
	is_attacking = false
	is_stunned = false
	is_frozen = false
	is_burning = false
	is_marked = false
	velocity = Vector2.ZERO
	$CollisionShape2D.disabled = true
	emit_signal("died")
	play_if_not("dead")

func play_if_not(anim_name: String):
	if $AnimatedSprite2D.animation != anim_name:
		$AnimatedSprite2D.play(anim_name)
		
func _on_stun_timer_timeout() -> void:
	if is_dead:
		return
	is_stunned = false
	
func _on_attack_timer_timeout() -> void:
	if is_dead:
		return
	
	deal_damage_to_player()
	is_attacking = false

func _on_freeze_timer_timeout() -> void:
	if is_dead:
		return
	
	is_frozen = false
	print("Inimigo descongelou")

func _on_burn_timer_timeout() -> void:
	if is_dead:
		return
	
	is_burning = false
	$BurnTickTimer.stop()
	print("Queimadura terminou")

func _on_burn_tick_timer_timeout() -> void:
	if is_dead:
		return
	
	if is_burning:
		take_damage(burn_damage)
		print("Queimadura causou ", burn_damage, " de dano")

func _on_mark_timer_timeout() -> void:
	if is_dead:
		return
	
	is_marked = false
	print("Marca expirou")

func _on_animated_sprite_2d_animation_finished() -> void:
	if $AnimatedSprite2D.animation == "dead":
		queue_free()
